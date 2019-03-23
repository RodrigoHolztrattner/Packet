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
	PacketLogger* _loggerPtr) :
	m_OperationMode(_operationMode), 
	m_ResourceStoragePtr(_storagePtr),
	m_FileLoaderPtr(_fileLoaderPtr), 
	m_ReferenceManagerPtr(_referenceManagerPtr),
	m_ResourceLoader(_fileLoaderPtr, _referenceManagerPtr, _loggerPtr, _operationMode),
	m_ResourceWatcherPtr(_resourceWatcherPtr), 
	m_Logger(_loggerPtr)
{
    if (m_OperationMode == OperationMode::Edit)
    {
        // Register the on resource data changed method for the resource watcher
        m_ResourceWatcherPtr->RegisterOnResourceDataChangedMethod(std::bind(&PacketResourceManager::OnResourceDataChanged, this, std::placeholders::_1));
    }
}

PacketResourceManager::~PacketResourceManager()
{
}

void PacketResourceManager::ReleaseObject(std::unique_ptr<PacketResourceInstance> _instancePtr)
{
	// Push the new release request, no need to synchronize this because we use a lock-free queue
    m_InstancesPendingReleaseEvaluation.enqueue(std::move(_instancePtr));
}

void PacketResourceManager::Update()
{
	// Prevent multiple threads from running this code (only one thread allowed, take care!)
	std::lock_guard<std::mutex> guard(m_Mutex);
}

void PacketResourceManager::AsynchronousResourceProcessment()
{
    // Call the update method for the resource watcher (disabled on non-edit builds)
    if (m_OperationMode == OperationMode::Edit)
    {
        m_ResourceWatcherPtr->Update();
    }

    ////////////////////////
    // EVALUATE INSTANCES //
    ////////////////////////
    while (true)
    {
        // If we have pending instances
        InstanceEvaluationData instanceEvaluationData;
        if (!m_InstancesPendingEvaluation.try_dequeue(instanceEvaluationData))
        {
            break;
        }

        // Get the info
        auto[instance, factory, buildInfo, hash, isPermanent, isRuntime, resourceData] = instanceEvaluationData;

        // Check if we need to create and load this resource
        PacketResource* resource = m_ResourceStoragePtr->FindObject(hash, buildInfo.buildFlags, isRuntime);
        if (resource == nullptr)
        {
            // Load this resource
            auto resourceUniquePtr = m_ResourceLoader.LoadObject(factory, 
                                                                 hash,
                                                                 buildInfo,
                                                                 isPermanent,
                                                                 isRuntime, 
                                                                 std::move(resourceData));
            resource = resourceUniquePtr.get();
            assert(resource != nullptr);

            // Watch this resource file object (not enabled on release and non-edit builds)
            m_ResourceWatcherPtr->WatchResource(resource);

            // Register this resource inside the storage
            m_ResourceStoragePtr->InsertObject(std::move(resourceUniquePtr), hash, buildInfo.buildFlags);

            // Construct the resource
            resource->OnConstruct();

            // If this resource requires external construct, enqueue it on the correspondent queue
            if (resource->RequiresExternalConstructPhase())
            {
                m_ResourcesPendingExternalConstruction.enqueue(resource);
            }
            // If no external synchronization is required, enqueue it on the post construction queue
            else
            {
                m_ResourcesPendingPostConstruction.enqueue(resource);
            }
        }

        // Make the instance reference it
        resource->MakeInstanceReference(instance);

        // Insert this instance into the wait vector
        m_InstancesPendingConstruction.push_back(instance);
    }

    ////////////////////////////////
    // RESOURCE POST CONSTRUCTION //
    ////////////////////////////////
    while (true)
    {
        // If we have pending resources
        PacketResource* resource;
        if (!m_ResourcesPendingPostConstruction.try_dequeue(resource))
        {
            break;
        }

        // Check if this resource was successfully created
        if (!resource->IsReady())
        {
            // Something bad happened, destroy this resource and inform all instances about that
            // ...

            // Release any reference on the pending replacement vector
            // TODO: Is this necessary?
        }
    }

    ///////////////////////////////////
    // RESOURCES PENDING REPLACEMENT //
    ///////////////////////////////////
    for (int i = static_cast<int>(m_ResourcesPendingReplacement.size() - 1); i >= 0; i--)
    {
        // Get a short variable to the resource
        auto& [newResource, originalResource] = m_ResourcesPendingReplacement[i];

        // Check if both the original and the new resources are ready (and if the instances were built)
        if (originalResource->IsReady() && newResource->IsReady() && originalResource->AreInstancesReadyToBeUsed())
        {
            // Update the resource watched
            m_ResourceWatcherPtr->UpdateWatchedResource(newResource.get());

            // Make all instances that points to the old resource now point to the new one
            originalResource->RedirectInstancesToResource(newResource.get());

            // Replace the new resource inside the storage (takes ownership)
            std::unique_ptr<PacketResource> oldResource = m_ResourceStoragePtr->ReplaceObject(
                newResource,
                newResource->GetHash(),
                newResource->GetBuildInfo().buildFlags);

            // Decrement the number of references for the original resource, now it
            // must have zero direct references since we moved all instances
            originalResource->DecrementNumberDirectReferences();
            assert(!originalResource->IsDirectlyReferenced());

            // Insert this resource into the deletion queue because it has no direct references (we moved all 
            // instances that referenced it into the new resource)
            m_ResourcesPendingDeletion.push_back(std::move(oldResource));

            // Remove the resource from this queue
            m_ResourcesPendingReplacement.erase(m_ResourcesPendingReplacement.begin() + i);
        }
    }

    ///////////////////////
    // RELEASE INSTANCES //
    ///////////////////////

    // We need to move all instances from the lock-free queue to our internal release vector
    while (true)
    {
        // If we have pending instances
        std::unique_ptr<PacketResourceInstance> instance;
        if (!m_InstancesPendingReleaseEvaluation.try_dequeue(instance))
        {
            break;
        }

        // Move this instance to our release vector
        m_InstancesPendingRelease.push_back(std::move(instance));
    }

    // For each instance pending release
    for (int i = static_cast<int>(m_InstancesPendingRelease.size() - 1); i >= 0; i--)
    {
        // Get the instance ptr
        std::unique_ptr<PacketResourceInstance>& instance = m_InstancesPendingRelease[i];

        // Verify is this instance is ready, we need to wait until it was successfully loaded to release it
        if (!instance->IsReady())
        {
            continue;
        }

        // Get a short variable to the internal resource
        PacketResource* resource = instance->GetResource();

        // Get this resource and instance factory
        PacketResourceFactory* factory = resource->GetFactoryPtr();

        // Remove the instance reference from this object
        resource->RemoveInstanceReference(instance.get());

        // Delete the instance using its factory object
        factory->ReleaseInstance(std::move(instance));

        // Check if the resource should be deleted
        if (!resource->IsDirectlyReferenced() && !resource->IsPermanent())
        {
            // Remove this object from the storage, taking its ownership back
            std::unique_ptr<PacketResource> objectUniquePtr = m_ResourceStoragePtr->GetObjectOwnership(resource);

            // Insert this object into the deletion queue
            m_ResourcesPendingDeletion.push_back(std::move(objectUniquePtr));
        }

        // Delete the current instance from this vector
        m_InstancesPendingRelease.erase(m_InstancesPendingRelease.begin() + i);
    }

    //////////////////////
    // DELETE RESOURCES //
    //////////////////////
    for (int i = static_cast<int>(m_ResourcesPendingDeletion.size() - 1); i >= 0; i--)
    {
        // Get a short variable to this resource
        auto& resource = m_ResourcesPendingDeletion[i];

        // Check if this resource is referenced in any way and if it is pending replacement
        if (resource->IsReferenced())
        {
            // We cannot delete it right now
            continue;
        }

        // Remove this resource watch (not enabled on release and non-edit builds)
        m_ResourceWatcherPtr->RemoveWatch(resource.get());

        // Set pending deletion for this resource
        resource->SetPendingDeletion();

        // Add this object into the deletion queue, takes ownership
        m_ResourceDeleter.DeleteObject(std::move(resource), resource->GetFactoryPtr());

        // Remove the resource from this queue
        m_ResourcesPendingDeletion.erase(m_ResourcesPendingDeletion.begin() + i);
    }

    /////////////////////
    // AWAKE INSTANCES //
    /////////////////////
    for (int i = static_cast<int>(m_InstancesPendingConstruction.size() - 1); i >= 0; i++)
    {
        // Get this instance resource
        PacketResource* resource = m_InstancesPendingConstruction[i]->GetResource();

        // Check if this resource is ready
        if (resource->IsReady())
        {
            // Call the construct method for this instance
            m_InstancesPendingConstruction[i]->BeginConstruction();

            // Remove this instance from the wait vector
            m_InstancesPendingConstruction.erase(m_InstancesPendingConstruction.begin() + i);
        }
    }

    // Take a little nap
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void PacketResourceManager::ReconstructInstance(PacketResourceInstance* _instance)
{

#ifndef NDEBUG

	// If we have this instance inside the construction queue, we have an error
	for (unsigned int i = 0; i < m_InstancesPendingConstruction.size(); i++)
	{
		// Compare the ptrs
		assert(m_InstancesPendingConstruction[i] != _instance);
	}

#endif

	// Insert the reference into the construct queue
    m_InstancesPendingConstruction.push_back(_instance);
}

void PacketResourceManager::OnResourceDataChanged(PacketResource* _resource)
{
	// There is a decent chance that this method was called by the same event multiple times, this happens because the internal 
	// implementation of the file notification system by the current operation system, to prevent this we need to check 3 
	// locations so we can be sure we won't be updating a resource when it is already being updated, those locations are: 
	// ReplaceQueue, DeletionQueue and if the current resource status inside the ResourceStorage is ready.

    // Disabled on non edit builds
    if (m_OperationMode != OperationMode::Edit)
    {
        return;
    }

	// Check if this resource ignore physical data changes
	if (_resource->IgnorePhysicalDataChanges())
	{
		return;
	}

	// Check if this resource is pending deletion
	if (_resource->IsPendingDeletion() || !_resource->IsDirectlyReferenced())
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
    for (auto& replaceInfo : m_ResourcesPendingReplacement)
    {
        if (replaceInfo.first->GetHash() == _resource->GetHash())
        {
            // There is no need to set it again, duplicated call
            m_Logger->LogWarning(std::string("Found duplicated file modification event on file: \"")
                                 .append(_resource->GetHash().GetPath().String())
                                 .append("\", ignoring it!")
                                 .c_str());

            return;
        }
    }

	// Also check the deletion queue
	for (auto& deletionRequest : m_ResourcesPendingDeletion)
	{
		// Compare the hashes
		if (deletionRequest->GetHash() == _resource->GetHash())
		{
			// There is no need to set it again, duplicated call
			m_Logger->LogWarning(std::string("Found file modification event on file: \"")
				.append(_resource->GetHash().GetPath().String())
				.append("\" but resource is being deleted, ignoring it!")
				.c_str());

			return;
		}
	}

    // Increment the number of references for the original resource (to prevent it from
    // being destroyed)
    _resource->IncrementNumberDirectReferences();

    // Load this resource
    auto resourceUniquePtr = m_ResourceLoader.LoadObject(_resource->GetFactoryPtr(), 
                                                         _resource->GetHash(),
                                                         _resource->GetBuildInfo(),
                                                         _resource->IsPermanent(), 
                                                         false,
                                                         {});

    // Construct the resource
    resourceUniquePtr->OnConstruct();

    // If this resource requires external construct, enqueue it on the correspondent queue
    if (resourceUniquePtr->RequiresExternalConstructPhase())
    {
        m_ResourcesPendingExternalConstruction.enqueue(resourceUniquePtr.get());
    }
    // If no external synchronization is required, enqueue it on the post construction queue
    else
    {
        m_ResourcesPendingPostConstruction.enqueue(resourceUniquePtr.get());
    }
    
	// Move the resource into the replace queue
    m_ResourcesPendingReplacement.push_back({ std::move(resourceUniquePtr), _resource });
}