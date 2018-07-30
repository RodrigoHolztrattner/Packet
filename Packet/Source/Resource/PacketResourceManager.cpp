////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceManager.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResourceManager.h"
#include "PacketResourceInstance.h"
#include <cassert>

// Using namespace Peasant
PacketUsingDevelopmentNamespace(Packet)

PacketResourceManager::PacketResourceManager(PacketResourceStorage* _storagePtr, PacketFileLoader* _fileLoaderPtr, uint32_t _workerThreads, ThreadIndexRetrieveMethod _threadIndexMethod) :
	m_ResourceStoragePtr(_storagePtr),
	m_ObjectLoader(_fileLoaderPtr)
{
	// If we have at last one worker thread
	if (_workerThreads > 1)
	{
		// Set the initial data
		m_ObjectRequests.AllowThreadedAccess(_workerThreads, _threadIndexMethod);
		m_InstanceReleases.AllowThreadedAccess(_workerThreads, _threadIndexMethod);
	}

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

void PacketResourceManager::ReleaseObject(PacketResource* _object, PacketResourceFactory* _factoryPtr, bool _allowAsynchronousDeletion)
{
	// Asserts
	assert(!m_InUpdatePhase || (m_InUpdatePhase && std::this_thread::get_id() == m_UpdateThreadID));

	// Create the new temporary release request
	TemporaryObjectRelease release = { _object, _factoryPtr, !_allowAsynchronousDeletion };

	// Push the new release request
	m_TemporaryInstanceReleases.Insert(release);
}

void PacketResourceManager::Update()
{
	// Prevent multiple threads from running this code (only one thread allowed, take care!)
	std::lock_guard<std::mutex> guard(m_Mutex);

	// Begin the update phase
	m_InUpdatePhase = true;
	m_UpdateThreadID = std::this_thread::get_id();

	// For each request, run the process method
	m_ObjectRequests.ProcessAll([&](ObjectRequest& _requestData)
	{
		// Check if we already have an object with this hash
		PacketResource* object = m_ResourceStoragePtr->FindObject(_requestData.hash);
		if (object == nullptr)
		{
			// Create a new object (use the factory)
			auto objectPtr = _requestData.factoryPtr->RequestObject();
			assert(objectPtr != nullptr);

			// Set the original variable ptr
			object = objectPtr.get();

			// Set the object hash
			object->SetHash(_requestData.hash);

			// Set the object factory reference
			object->SetFactoryReference(_requestData.factoryPtr);

			// Insert this file into the load queue
			m_ObjectLoader.LoadObject(object, _requestData.hash, _requestData.isPermanent);

			// Insert the new object into the storage (takes ownership)
			m_ResourceStoragePtr->InsertObject(objectPtr, _requestData.hash);
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
		// its delleting was called, we need to wait until it is ready to be used so we can safelly delete it
		if (!_releaseRequest.instance->IsReady())
		{
			// Ignore this instance until it's ready to be used
			return;
		}

		// Get a short variable to the instance object
		auto* object = _releaseRequest.instance->GetObjectPtr();

		// Delete the instance using its factory object
		_releaseRequest.factoryPtr->ReleaseInstance(_releaseRequest.instance);

		// Release this instance
		object->ReleaseInstance();

		// Check if the object should be deleted
		if (!object->IsReferenced() && !object->IsPersistent())
		{
			// Remove this object from the storage, taking its ownership back
			std::unique_ptr<PacketResource> objectUniquePtr = m_ResourceStoragePtr->GetObjectOwnership(object);

			// Add this object into the deletion queue, takes ownership
			m_ObjectDeleter.DeleteObject(objectUniquePtr, _releaseRequest.factoryPtr, _releaseRequest.deleteSync);
		}

		// Delete the current instance from this queue
		m_InstanceReleases.DeleteProcessingObject();

	}, false);

	/* How we are going to do this?
	// For each temporary release, run the process method
	m_TemporaryInstanceReleases.ProcessAll([&](TemporaryObjectRelease _releaseRequest)
	{
		// Release this instance
		_releaseRequest.object->ReleaseInstance();

		// Check if the object should be deleted
		if (!_releaseRequest.object->IsReferenced() && !_releaseRequest.object->IsPersistent())
		{
			// Remove this object from the storage
			m_ResourceStoragePtr->RemoveObject(_releaseRequest.object);

			// Add this object into the deletion queue
			m_ObjectDeleter.DeleteObject(_releaseRequest.object, _releaseRequest.factoryPtr, _releaseRequest.deleteSync);
		}

	}, true);
	*/

	// Process all construct objects
	for (unsigned int i = 0; i < m_ConstructQueue.size(); i++)
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

			// Return 1 from the current index
			i--;
		}
	}

	// Call the update method for the object deleter and loader
	m_ObjectDeleter.Update();
	m_ObjectLoader.Update();

	// End the update phase
	m_InUpdatePhase = false;
}

std::map<HashPrimitive, std::unique_ptr<PacketResource>>& PacketResourceManager::QueryLoadedObjects()
{
	return m_ResourceStoragePtr->GetObjectMapReference();
}