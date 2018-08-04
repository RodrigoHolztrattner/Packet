////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceManager.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResourceManager.h"
#include "PacketResourceInstance.h"
#include "PacketResourceFactory.h"
#include "PacketResourceWatcher.h"

#include <cassert>

// Using namespace Peasant
PacketUsingDevelopmentNamespace(Packet)

PacketResourceManager::PacketResourceManager(OperationMode _operationMode, 
	PacketResourceStorage* _storagePtr,
	PacketFileLoader* _fileLoaderPtr, 
	PacketReferenceManager* _referenceManagerPtr, 
	PacketResourceWatcher* _resourceWatcherPtr, 
	PacketLogger* _loggerPtr, 
	uint32_t _workerThreads, 
	ThreadIndexRetrieveMethod _threadIndexMethod) :
	m_OperationMode(_operationMode), 
	m_ResourceStoragePtr(_storagePtr),
	m_FileLoaderPtr(_fileLoaderPtr), 
	m_ReferenceManagerPtr(_referenceManagerPtr),
	m_ResourceLoader(_fileLoaderPtr, _referenceManagerPtr, _loggerPtr, _operationMode),
	m_ResourceWatcherPtr(_resourceWatcherPtr), 
	m_Logger(_loggerPtr)
{
	// If we have at last one worker thread
	if (_workerThreads > 1)
	{
		// Set the initial data
		m_ResourceRequests.AllowThreadedAccess(_workerThreads, _threadIndexMethod);
		m_InstanceReleases.AllowThreadedAccess(_workerThreads, _threadIndexMethod);
	}

	// Register the on resource data changed method for the resource watcher
	m_ResourceWatcherPtr->RegisterOnResourceDataChangedMethod(std::bind(&PacketResourceManager::OnResourceDataChanged, this, std::placeholders::_1));

	// Set the initial data
	m_InUpdatePhase = false;
}

PacketResourceManager::~PacketResourceManager()
{
}

void PacketResourceManager::ReleaseObject(std::unique_ptr<PacketResourceInstance>& _instancePtr, PacketResourceFactory* _factoryPtr, bool _allowAsynchronousDeletion)
{
	// Asserts
	assert(!m_InUpdatePhase || (m_InUpdatePhase && std::this_thread::get_id() == m_UpdateThreadID));

	// Create the new release request
	ObjectRelease release = { std::move(_instancePtr), _factoryPtr, !_allowAsynchronousDeletion };

	// Push the new release request
	m_InstanceReleases.Insert(std::move(release));
}

void PacketResourceManager::Update()
{
	// Prevent multiple threads from running this code (only one thread allowed, take care!)
	std::lock_guard<std::mutex> guard(m_Mutex);

	// Begin the update phase
	m_InUpdatePhase = true;
	m_UpdateThreadID = std::this_thread::get_id();

	// For each request, run the process method
	m_ResourceRequests.ProcessAll([&](ObjectRequest& _requestData)
	{
		// Check if we already have an object with this hash
		PacketResource* object = m_ResourceStoragePtr->FindObject(_requestData.hash, _requestData.buildInfo.buildFlags);
		if (object == nullptr)
		{
			// Create a new object (use the factory)
			auto objectPtr = _requestData.factoryPtr->RequestObject();
			assert(objectPtr != nullptr);

			// Set the original variable ptr
			object = objectPtr.get();

			// Set the object hash
			object->SetHash(_requestData.hash);

			// Set the helper pointers and the current operation mode
			object->SetHelperObjects(_requestData.factoryPtr, m_ReferenceManagerPtr, m_FileLoaderPtr, m_Logger, m_OperationMode);

			// Set the build info
			object->SetBuildInfo(_requestData.buildInfo);
			
			// Watch this resource file object (not enabled on release and non-edit builds)
			m_ResourceWatcherPtr->WatchResource(object);

			// Insert this file into the load queue
			m_ResourceLoader.LoadObject(object, _requestData.hash, _requestData.isPermanent);

			// Insert the new object into the storage (takes ownership)
			m_ResourceStoragePtr->InsertObject(objectPtr, _requestData.hash, _requestData.buildInfo.buildFlags);
		}

		// Make the instance reference it
		object->MakeInstanceReference(_requestData.instance);

		// Insert the reference into the construct queue
		m_ConstructQueue.push_back(_requestData.instance);

	}, true);

	// For each release, run the process method
	m_InstanceReleases.ProcessAll([&](ObjectRelease& _releaseRequest)
	{
		// Check if this instance is ready to be used (it's possible that it is still being created/loaded and 
		// its deletion was requested, we need to wait until it is ready to be used so we can safelly delete it
		if (!_releaseRequest.instance->IsReady())
		{
			// Ignore this instance until it's ready to be used
			return;
		}

		// Get a short variable to the instance object
		auto* object = _releaseRequest.instance->GetObjectPtr();

		// Remove the instance reference from this object
		object->RemoveInstanceReference(_releaseRequest.instance.get());

		// Delete the instance using its factory object
		_releaseRequest.factoryPtr->ReleaseInstance(_releaseRequest.instance);

		// Check if the object should be deleted
		if (!object->IsDirectlyReferenced() && !object->IsPersistent())
		{
			// Remove this object from the storage, taking its ownership back
			std::unique_ptr<PacketResource> objectUniquePtr = m_ResourceStoragePtr->GetObjectOwnership(object);

			// Insert this object into the deletion queue
			m_DeletionQueue.push_back({ std::move(objectUniquePtr), _releaseRequest.deleteSync });
		}

		// Delete the current instance from this queue
		m_InstanceReleases.DeleteProcessingObject();

	}, false);

	// For each resource on our deletion queue (do it backwards to easy deletion)
	for (int i = int(m_DeletionQueue.size()) - 1; i >= 0; i--)
	{
		// Get the deletion request
		auto& deletionRequest = m_DeletionQueue[i];

		// Check if the resource is referenced by any instance or temporary instances (use the common method, not the specific ones), 
		// also check if it is pending replacement
		if (!deletionRequest.resource->IsReferenced() && !deletionRequest.resource->IsPendingReplacement())
		{
			// Remove this resource watch (not enabled on release and non-edit builds)
			m_ResourceWatcherPtr->RemoveWatch(deletionRequest.resource.get());

			// Set pending deletion for this resource
			deletionRequest.resource->SetPendingDeletion();

			// Add this object into the deletion queue, takes ownership
			m_ResourceDeleter.DeleteObject(deletionRequest.resource, deletionRequest.resource->GetFactoryPtr(), deletionRequest.deleteSync);

			// Remove the resource from this queue
			m_DeletionQueue.erase(m_DeletionQueue.begin() + i);
		}
	}

	// Process all construct objects (do it backwards to easy deletion)
	for (int i = int(m_ConstructQueue.size()) - 1; i >= 0; i--)
	{
		// Get the instance
		PacketResourceInstance* instance = m_ConstructQueue[i];

		// Get the resource reference
		auto* resource = instance->GetObjectPtr();

		// Check if the internal object was loaded (and synchronized) and this instance is ready to be constructed
		if (resource->IsReady())
		{
			// Construct this instance
			instance->BeginConstruction();

			// Remove it from the construct queue
			m_ConstructQueue.erase(m_ConstructQueue.begin() + i);
		}
	}

	// For each resource on the replacement queue (there should be none here when not on debug builds)
	// (do it backwards to easy deletion)
	for (int i = int(m_ReplaceQueue.size()) - 1; i >= 0; i--)
	{
		// Get the resource
		PacketResource* resource = m_ReplaceQueue[i];

		// Check if all instances are totally constructed and ready to be used, so we can proceed without problems
		if (!resource->AreInstancesReadyToBeUsed())
		{
			// Ignore this resource for now
			continue;
		}

		// Get this resource factory
		auto* factory = resource->GetFactoryPtr();

		// Get the resource hash
		Hash hash = resource->GetHash();

		// Create a new object (use the factory)
		auto objectPtr = factory->RequestObject();
		assert(objectPtr != nullptr);

		// Set the object hash
		objectPtr->SetHash(hash);

		// Set the helper pointers and the current operation mode
		objectPtr->SetHelperObjects(factory, m_ReferenceManagerPtr, m_FileLoaderPtr, m_Logger, m_OperationMode);

		// Set the build info
		objectPtr->SetBuildInfo(resource->GetBuildInfo());

		// Update the resource watched
		m_ResourceWatcherPtr->UpdateWatchedResource(objectPtr.get());

		// Make all instances that points to the old resource now point to the new one
		resource->RedirectInstancesToResource(objectPtr.get());

		// Insert this file into the load queue
		m_ResourceLoader.LoadObject(objectPtr.get(), hash, resource->IsPersistent());

		// Replace the new resource inside the storage (takes ownership)
		std::unique_ptr<PacketResource> oldResource = m_ResourceStoragePtr->ReplaceObject(objectPtr, hash, resource->GetBuildInfo().buildFlags);

		// Insert this resource into the deletion queue because it has no direct references (we moved all 
		// instances that referenced it into the new resource)
		m_DeletionQueue.push_back({ std::move(oldResource), resource->GetBuildInfo().asyncResourceObjectDeletion });

		// Remove the resource from this queue
		m_ReplaceQueue.erase(m_ReplaceQueue.begin() + i);
	}

	// Call the update method for the object deleter and loader
	m_ResourceDeleter.Update();
	m_ResourceLoader.Update();

#ifndef NDEBUG

	// Call the update method for the resource watcher (disabled on release and non-edit builds)
	m_ResourceWatcherPtr->Update();

#endif

	// End the update phase
	m_InUpdatePhase = false;
}

void PacketResourceManager::ReconstructInstance(PacketResourceInstance* _instance)
{

#ifndef NDEBUG

	// If we have this instance inside the construction queue, we have an error
	for (unsigned int i = 0; i < m_ConstructQueue.size(); i++)
	{
		// Compare the ptrs
		assert(m_ConstructQueue[i] != _instance);
	}

#endif

	// Insert the reference into the construct queue
	m_ConstructQueue.push_back(_instance);
}

void PacketResourceManager::OnResourceDataChanged(PacketResource* _resource)
{
	// There is a decent chance that this method was called by the same event multiple times, this happens because the internal 
	// implementation of the file notification system by the current operation system, to prevent this we need to check 3 
	// locations so we can be sure we won't be updating a resource when it is already being updated, those locations are: 
	// ReplaceQueue, DeletionQueue and if the current resource status inside the ResourceStorage is ready.

	// Check if this resource ignore physical data changes
	if (_resource->IgnorePhysicalDataChanges())
	{
		return;
	}

	// Check if this resource is pending deletion
	if (_resource->IsPendingDeletion())
	{
		// We can't proceed with this resource on its current status
		m_Logger->LogWarning(std::string("Found file modification event on file: \"")
			.append(_resource->GetHash().GetPath().String())
			.append("\", but the resource is pending deletion, ignoring this!")
			.c_str());

		return;
	}

	// Check if we already have this resource inside the replace queue (this shouldn't be slow because 
	// we aren't supposed to have multiple resources here)
	// Check
	if (std::find(m_ReplaceQueue.begin(), m_ReplaceQueue.end(), _resource) != m_ReplaceQueue.end())
	{
		// There is no need to set it again, duplicated call
		m_Logger->LogWarning(std::string("Found duplicated file modification event on file: \"")
			.append(_resource->GetHash().GetPath().String())
			.append("\", ignoring it!")
			.c_str());

		return;
	}

	// Also check the deletion queue
	for (auto& deletionRequest : m_DeletionQueue)
	{
		// Compare the hashes
		if (deletionRequest.resource->GetHash() == _resource->GetHash())
		{
			// There is no need to set it again, duplicated call
			m_Logger->LogWarning(std::string("Found duplicated file modification event on file: \"")
				.append(_resource->GetHash().GetPath().String())
				.append("\", ignoring it!")
				.c_str());

			return;
		}
	}

	// Get the resource object from the storage and check if it is ready
	auto* resource = m_ResourceStoragePtr->FindObject(_resource->GetHash(), _resource->GetBuildInfo().buildFlags);
	if (!resource->IsReady())
	{
		// There is no need to set it again, duplicated call
		m_Logger->LogWarning(std::string("Found duplicated file modification event on file: \"")
			.append(_resource->GetHash().GetPath().String())
			.append("\", ignoring it!")
			.c_str());

		return;
	}

	// Set that this resource is now pending replacement
	_resource->SetPedingReplacement();

	// Insert the resource into the replace queue
	m_ReplaceQueue.push_back(_resource);
}