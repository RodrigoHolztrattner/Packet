////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceManager.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResourceManager.h"
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
	m_ResourceLoader(_fileLoaderPtr, _referenceManagerPtr, this, _loggerPtr, _operationMode),
	m_ResourceWatcherPtr(_resourceWatcherPtr), 
	m_Logger(_loggerPtr)
{
    if (m_OperationMode == OperationMode::Edit)
    {
        // Register the on resource data changed method for the resource watcher
        m_ResourceWatcherPtr->RegisterOnResourceDataChangedMethod(std::bind(&PacketResourceManager::OnResourceDataChanged, this, std::placeholders::_1));
    }

    // Create the asynchronous thread that will process instances and resource objects
    m_AsynchronousManagementThread = std::thread([&]()
    {
        while (!m_AsynchronousManagementThreadShouldExit)
        {
            AsynchronousResourceProcessment();

            std::this_thread::sleep_for(std::chrono::milliseconds(ThreadSleepTimeMS));
        }
    });
}

PacketResourceManager::~PacketResourceManager()
{
    // Exit from the asynchronous thread
    m_AsynchronousManagementThreadShouldExit = true;
    m_AsynchronousManagementThread.join();

    // Resource create proxies that are pending evaluation
    {
        // Sorry but no proxy will be evaluated, just dequeue all of them
        ResourceCreationData resourceCreationData;
        while (m_ResourceCreateProxyQueue.try_dequeue(resourceCreationData))
        {
        }
    }

    // Permanent resources
    {
        // Get all permanent resources and move them to the delete vector
        std::vector<std::unique_ptr<PacketResource>> permanentResources = m_ResourceStoragePtr->GetPermanentResourcesOwnership();
        m_ResourcesPendingDeletion.insert(m_ResourcesPendingDeletion.end(),
                                          std::make_move_iterator(permanentResources.begin()),
                                          std::make_move_iterator(permanentResources.end()));
    }

    // For each resource pending replacement
    {
        for (auto& replacementInfo : m_ResourcesPendingReplacement)
        {
            auto&[newResourceUniquePtr, originalResource] = replacementInfo;

            // Get the original resource ownership
            auto originalResourceUniquePtr = m_ResourceStoragePtr->GetObjectOwnership(originalResource);

            // Decrement the number of references for the original resource, now it
            // must have zero references
            originalResourceUniquePtr->DecrementNumberReferences(false);
            assert(!originalResourceUniquePtr->IsReferenced());

            // Decrement a reference for the new resource, this reference was added when
            // it was created and we must remove it to reach zero references
            newResourceUniquePtr->DecrementNumberReferences(false);
            assert(!newResourceUniquePtr->IsReferenced());

            // Insert both resources into the deletion queue because
            m_ResourcesPendingDeletion.push_back(std::move(originalResourceUniquePtr));
            m_ResourcesPendingDeletion.push_back(std::move(newResourceUniquePtr));
        }

        m_ResourcesPendingReplacement.clear();
    }

    // Resources pending construction
    {
        // Sorry but we are closed today
        PacketResource* resource;
        while (m_ResourcesPendingExternalConstruction.try_dequeue(resource))
        {
            // Get the original resource ownership
            auto resourceUniquePtr = m_ResourceStoragePtr->GetObjectOwnership(resource);

            // Set it to be destroyed right below
            m_ResourcesPendingDeletion.push_back(std::move(resourceUniquePtr));
        } 
    }
    
    // Resources pending modifications
    {
        // We don't need to do anything about this pointer
        PacketResource* resource;
        while (m_ResourcesPendingModificationEvaluation.try_dequeue(resource))
        {
        }
    }

    // Remove reference count for replacing resources
    for (auto& resource : m_ResourcesPendingDeletion)
    {
        // If this resource was replaced by another, we must remove one reference from the replacing
        // resource here
        if (resource->GetReplacingResource())
        {
            resource->GetReplacingResource().value()->DecrementNumberReferences();
        }
    }

    // Gather all resources on the deletion evaluation queue and move to the deletion vector
    while (true)
    {
        PacketResource* resourcePtr;
        if (!m_ResourcesPendingDeletionEvaluation.try_dequeue(resourcePtr))
        {
            break;
        }

        // Remove this object from the storage, taking its ownership back
        std::unique_ptr<PacketResource> objectUniquePtr = m_ResourceStoragePtr->GetObjectOwnership(resourcePtr);

        // Move this resource to our deletion vector
        m_ResourcesPendingDeletion.push_back(std::move(objectUniquePtr));
    }

    // Resources pending deletion
    {
        // Just normally delete these resources
        for (auto& resource : m_ResourcesPendingDeletion)
        {
            // This resource can't be referenced, if it has references this means that some instance or resource 
            // reference is still active when the packet system was set to shutdown, the correct behavior is
            // to release/destroy every other object before releasing the main system
            assert(!resource->IsReferenced() && "Packet resource system is being deleted but some resources are \
 still active (they have instances and/or resource references), this will probably lead into exceptions!");

            // Remove this resource watch (not enabled on release and non-edit builds)
            m_ResourceWatcherPtr->RemoveWatch(resource.get());

            // Set pending deletion for this resource
            resource->SetPendingDeletion();

            // Add this object into the deletion queue, takes ownership
            m_ResourceDeleter.DeleteObject(std::move(resource), resource->GetFactoryPtr());
        }

        m_ResourcesPendingDeletion.clear();
    }

    assert(m_ResourceCreateProxyQueue.size_approx()               == 0);
    assert(m_ResourcesPendingExternalConstruction.size_approx()   == 0);
    assert(m_ResourcesPendingDeletionEvaluation.size_approx()     == 0);
    assert(m_ResourcesPendingDeletion.size()                      == 0);
    assert(m_ResourcesPendingReplacement.size()                   == 0);
    assert(m_ResourcesPendingModificationEvaluation.size_approx() == 0);
    assert(m_ResourcesPendingModification.size()                  == 0);
}


std::vector<PacketResourceExternalConstructor> PacketResourceManager::GetResourceExternalConstructors()
{
    std::vector<PacketResourceExternalConstructor> outConstructors;

    PacketResource* resource = nullptr;
    while (m_ResourcesPendingExternalConstruction.try_dequeue(resource))
    {
        outConstructors.emplace_back(PacketResourceExternalConstructor(resource, this));
    }

    return std::move(outConstructors);
}

void PacketResourceManager::RegisterResourceForDeletion(PacketResource* _resourcePtr)
{
    // If this is a permanent resource, do not register it
    if (!_resourcePtr->IsPermanent())
    {
        m_ResourcesPendingDeletionEvaluation.enqueue(_resourcePtr);
    }
}

std::unique_ptr<PacketResourceCreationProxy> PacketResourceManager::GetResourceCreationProxy()
{
    std::unique_ptr<PacketResourceCreationProxy> proxy;
    if (!m_ResourceCreateProxyFreeQueue.try_dequeue(proxy))
    {
        proxy = std::make_unique<PacketResourceCreationProxy>();
    }

    return std::move(proxy);
}

uint32_t PacketResourceManager::GetApproximatedNumberResourcesPendingDeletion()
{
    return static_cast<uint32_t>(m_ResourcesPendingDeletion.size());
}

void PacketResourceManager::AsynchronousResourceProcessment()
{
    // Call the update method for the resource watcher (disabled on non-edit builds)
    if (m_OperationMode == OperationMode::Edit)
    {
        m_ResourceWatcherPtr->Update();
    }

    /////////////////////////////
    // EVALUATE CREATION DATAS //
    /////////////////////////////
    while (true)
    {
        // If we have pending instances
        ResourceCreationData resourceCreationData;
        if (!m_ResourceCreateProxyQueue.try_dequeue(resourceCreationData))
        {
            break;
        }

        // Get the info
        auto&[creationProxy, factory, buildInfo, hash, isPermanent, isRuntime, resourceData] = resourceCreationData;

        // Check if we need to create and load this resource
        PacketResource* resource = m_ResourceStoragePtr->FindObject(hash, buildInfo.buildFlags, isRuntime);
        if (resource == nullptr)
        {
            // Load this resource
            auto resourceUniquePtr = m_ResourceLoader.LoadObject(
                factory,
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

            // If this resource requires external construct, enqueue it on the correspondent queue
            if (resource->RequiresExternalConstructPhase() && !resource->ConstructionFailed())
            {
                m_ResourcesPendingExternalConstruction.enqueue(resource);
            }
        }

        // Link the reference
        creationProxy->ForwardResourceLink(resource);

        // Insert the proxy into the free queue to be reused
        m_ResourceCreateProxyFreeQueue.enqueue(std::move(creationProxy));
    }

    ///////////////////////////////////
    // RESOURCES PENDING REPLACEMENT //
    ///////////////////////////////////
    for (int i = static_cast<int>(m_ResourcesPendingReplacement.size() - 1); i >= 0; i--)
    {
        // Get a short variable to the resource
        auto& [newResource, originalResource] = m_ResourcesPendingReplacement[i];

        // Check if both the original and the new resources are ready
        if (originalResource->IsValid() && newResource->IsValid())
        {
            // Update the resource watched
            m_ResourceWatcherPtr->UpdateWatchedResource(newResource.get());

            // Register the replacing resource
            originalResource->RegisterReplacingResource(newResource.get());

            // Replace the new resource inside the storage (takes ownership)
            std::unique_ptr<PacketResource> oldResource = m_ResourceStoragePtr->ReplaceObject(
                newResource,
                newResource->GetHash(),
                newResource->GetBuildInfo().buildFlags);

            // Decrement the number of references for the original resource, now it
            // must have zero references since we moved all references
            originalResource->DecrementNumberReferences();

            // Insert this resource into the deletion queue, it won't be deleted until all references
            // start referencing the new resource so its safe to put it here
            m_ResourcesPendingDeletion.push_back(std::move(oldResource));

            // Remove the resource from this queue
            m_ResourcesPendingReplacement.erase(m_ResourcesPendingReplacement.begin() + i);
        }
        // Check if the new resource failed to be constructed
        else if (newResource->ConstructionFailed())
        {
            // Remove the reference used to prevent the deletion of the original and the new resources
            originalResource->DecrementNumberReferences();
            newResource->DecrementNumberReferences();

            // Do not replace the resource since the new one failed, delete the new resource
            m_ResourcesPendingDeletion.push_back(std::move(newResource));

            // Remove the resource from this queue
            m_ResourcesPendingReplacement.erase(m_ResourcesPendingReplacement.begin() + i);
        }
    }

    /////////////////////////////////////
    // RESOURCES PENDING MODIFICATIONS // 
    /////////////////////////////////////
    while (true)
    {
        // If we have resourced pending modifications
        PacketResource* resource;
        if (!m_ResourcesPendingModificationEvaluation.try_dequeue(resource))
        {
            break;
        }

        m_ResourcesPendingModification.push_back(resource);
    }

    // For each resource pending modification
    for (int i = static_cast<int>(m_ResourcesPendingModification.size() - 1); i >= 0; i--)
    {
        PacketResource* resource = m_ResourcesPendingModification[i];

        // Verify is this resource doesn't have references
        if (resource->IsReferenced())
        {
            continue;
        }

        // Begin the modifications
        resource->BeginModifications();

        m_ResourcesPendingModification.erase(m_ResourcesPendingModification.begin() + i);
    }

    ////////////////////////////////
    // RESOURCES PENDING DELETION //
    ////////////////////////////////

    // Gather all resources on the deletion evaluation queue and move to the deletion vector
    while (true)
    {
        PacketResource* resourcePtr;
        if (!m_ResourcesPendingDeletionEvaluation.try_dequeue(resourcePtr))
        {
            break;
        }

        // Check if this resource didn't acquire any reference after it was set to be released, this
        // could happened if two references requested it, it was created and for some reason the first
        // reference was released before acquiring a reference to it, that would cause the resource to
        // be enqueued to deletion even if the second reference wanna use it
        if (resourcePtr->IsReferenced())
        {
            // Just ignore this resource
            continue;
        }

        // Remove this object from the storage, taking its ownership back
        std::unique_ptr<PacketResource> objectUniquePtr = m_ResourceStoragePtr->GetObjectOwnership(resourcePtr);

        // Remove this resource watch (not enabled on release and non-edit builds)
        m_ResourceWatcherPtr->RemoveWatch(objectUniquePtr.get());

        // Move this resource to our deletion vector
        m_ResourcesPendingDeletion.push_back(std::move(objectUniquePtr));
    }

    // For each resource pending deletion
    for (int i = static_cast<int>(m_ResourcesPendingDeletion.size() - 1); i >= 0; i--)
    {
        // Get the resource ptr
        std::unique_ptr<PacketResource>& resourceUniquePtr = m_ResourcesPendingDeletion[i];

        // Verify is this resource is ready, if not we need to wait until it is totally evaluated
        if (!resourceUniquePtr->IsValid())
        {
            continue;
        }

        // If this resource was replaced by another, it could have some references still referencing
        // it, we won't be able to delete the resource until all of them reference the new resource
        if (resourceUniquePtr->GetReplacingResource() && resourceUniquePtr->IsReferenced())
        {
            // Ignore this resource for now
            continue;
        }

        assert(!resourceUniquePtr->IsReferenced());

        // If this resource was replaced by another, we must remove one reference from the replacing
        // resource here
        if (resourceUniquePtr->GetReplacingResource())
        {
            resourceUniquePtr->GetReplacingResource().value()->DecrementNumberReferences();
        }

        // Set pending deletion for this resource
        resourceUniquePtr->SetPendingDeletion();

        // Add this object into the deletion queue, takes ownership
        m_ResourceDeleter.DeleteObject(std::move(resourceUniquePtr), resourceUniquePtr->GetFactoryPtr());

        // Remove the resource from this queue
        m_ResourcesPendingDeletion.erase(m_ResourcesPendingDeletion.begin() + i);
    }

    // Take a little nap
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void PacketResourceManager::RegisterResourceForModifications(PacketResource* _resource)
{
    m_ResourcesPendingModificationEvaluation.enqueue(_resource);
}

void PacketResourceManager::RegisterResourceForExternalConstruction(PacketResource* _resource)
{
    m_ResourcesPendingExternalConstruction.enqueue(_resource);
}

void PacketResourceManager::OnResourceDataChanged(PacketResource* _resource)
{
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
	if (_resource->IsPendingDeletion() || !_resource->IsReferenced())
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
        if (replaceInfo.second->GetHash() == _resource->GetHash())
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

    // Check if the old resource already have a replacing resource
    if (_resource->GetReplacingResource())
    {
        // There is no need to set it again, duplicated call
        m_Logger->LogWarning(std::string("Found file modification event on file: \"")
                             .append(_resource->GetHash().GetPath().String())
                             .append("\" but resource is already replaced, ignoring it!")
                             .c_str());
        return;
    }

    // Increment the number of references for the original resource (to prevent it from
    // being destroyed)
    _resource->IncrementNumberReferences();

    // Sometimes when the resource was modified it is still in process of being saved when we get the
    // event here, to prevent a failure when loading this file we must wait until it can be detected
    uint32_t maxAttempts = 10;
    while (maxAttempts-- != 0)
    {
        const char* path = _resource->GetHash().GetPath().String();
        if (std::filesystem::exists(path) && !std::filesystem::is_directory(path))
        {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(3 * std::min(1, 10 - (int)maxAttempts)));
    }

    // Load this resource
    auto resourceUniquePtr = m_ResourceLoader.LoadObject(_resource->GetFactoryPtr(), 
                                                         _resource->GetHash(),
                                                         _resource->GetBuildInfo(),
                                                         _resource->IsPermanent(), 
                                                         false,
                                                         {});

    // Increment the number of references of this resource, since it will be replacing the old resource
    // when this very resource is deleted it will remove this added reference
    resourceUniquePtr->IncrementNumberReferences();

    // If this resource requires external construct, enqueue it on the correspondent queue
    if (resourceUniquePtr->RequiresExternalConstructPhase() && !resourceUniquePtr->ConstructionFailed())
    {
        m_ResourcesPendingExternalConstruction.enqueue(resourceUniquePtr.get());
    }
    else if(!resourceUniquePtr->ConstructionFailed())
    {
        // If this resource doesn't need external construct, call it here to set the internal flags
        resourceUniquePtr->BeginExternalConstruct(nullptr);
    }
    
	// Move the resource into the replace queue
    m_ResourcesPendingReplacement.push_back({ std::move(resourceUniquePtr), _resource });
}