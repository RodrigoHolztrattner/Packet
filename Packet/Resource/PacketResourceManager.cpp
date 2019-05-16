////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceManager.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResourceManager.h"

#include <cassert>

// Using namespace Peasant
PacketUsingDevelopmentNamespace(Packet)

PacketResourceManager::PacketResourceManager(
    OperationMode            _operationMode,
    PacketResourceStorage&   _storage,
    const PacketFileLoader&  _fileLoader,
    PacketFileIndexer& _fileIndexer,
    PacketLogger*            _loggerPtr) :
    m_OperationMode(_operationMode),
    m_ResourceStorage(_storage),
    m_FileLoader(_fileLoader),
    m_FileIndexer(_fileIndexer), 
    m_ResourceLoader(_fileLoader, *this, _loggerPtr, _operationMode),
    m_LoggerPtr(_loggerPtr)
{
    // Create the asynchronous thread that will process instances and resource objects
    m_AsynchronousManagementThread = std::thread([&]()
    {
        while (!m_AsynchronousManagementThreadShouldExit)
        {
            AsynchronousResourceProcessment();

            std::this_thread::sleep_for(std::chrono::milliseconds(ThreadSleepTimeMS));
        }
    });

    // If we are on plain mode, register the callback for when a file is modified, this
    // will make sure we update any necessary resource, if applicable
    if (m_OperationMode == OperationMode::Plain)
    {
        m_FileIndexer.RegisterFileModificationCallback(
            [&](const Path& _path)
            {
                // Register this path hash
                m_ModifiedFiles.enqueue(Hash(_path));
            });
    }
}

PacketResourceManager::~PacketResourceManager()
{
    // Exit from the asynchronous thread
    m_AsynchronousManagementThreadShouldExit = true;
    m_AsynchronousManagementThread.join();

    // Empty the file modification queue
    {
        HashPrimitive resource_hash;
        while (m_ModifiedFiles.try_dequeue(resource_hash))
        {

        }
    }

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
        std::vector<std::unique_ptr<PacketResource>> permanentResources = m_ResourceStorage.GetPermanentResourcesOwnership();
        m_ResourcesPendingDeletion.insert(m_ResourcesPendingDeletion.end(),
                                          std::make_move_iterator(permanentResources.begin()),
                                          std::make_move_iterator(permanentResources.end()));
    }

    // For each resource pending replacement
    {
        for (auto& replacementInfo : m_ResourcesPendingReplacement)
        {
            auto& [newResourceUniquePtr, originalResource] = replacementInfo;

            // Get the original resource ownership
            auto originalResourceUniquePtr = m_ResourceStorage.GetObjectOwnership(originalResource);

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
        // Sorry but we are closed today, do nothing with these requests since
        // all references should be already released by now and their respective
        // resources should already be on the deletion vector
        PacketResource* resource;
        while (m_ResourcesPendingExternalConstruction.try_dequeue(resource))
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

    ///////////////////////
    // RESOURCE DELETION //
    ///////////////////////

    // Since resources could have references to another resources and we will only release these references if the
    // first one is destroyed, we must keep checking if there is a new resource on the m_ResourcesPendingDeletionEvaluation
    // queue because that is the location new resources are going to be put in the above happens
    bool resourceDeletionIsStable = true;
    do
    {
        // Reset the do-while check variable
        resourceDeletionIsStable = true;

        // Gather all resources on the deletion evaluation queue and move to the deletion vector
        while (true)
        {
            PacketResource* resourcePtr;
            if (!m_ResourcesPendingDeletionEvaluation.try_dequeue(resourcePtr))
            {
                break;
            }

            // Remove this object from the storage, taking its ownership back
            std::unique_ptr<PacketResource> objectUniquePtr = m_ResourceStorage.GetObjectOwnership(resourcePtr);
            if (objectUniquePtr == nullptr)
            {
                // This resource was already set to be deleted, this happened because it was a permanent resource that
                // another resource had a dependency on, all permanent resource were moved to the m_ResourcesPendingDeletion
                // vector (so their ownership was already taken) but the resource couldn't be deleted because its parent
                // still had a reference to it, after looping a few times the parent finally released this resource and
                // it was added to this queue (m_ResourcesPendingDeletionEvaluation) but its unique_ptr is already on the
                // m_ResourcesPendingDeletion vector waiting to be destroyed, we don't need to do anything
                continue;
            }

            // Move this resource to our deletion vector
            m_ResourcesPendingDeletion.push_back(std::move(objectUniquePtr));
        }

        // Resources pending deletion
        {
            // For each resource pending deletion
            for (int i = static_cast<int>(m_ResourcesPendingDeletion.size() - 1); i >= 0; i--)
            {
                // Get the resource ptr
                std::unique_ptr<PacketResource>& resource = m_ResourcesPendingDeletion[i];

                // If this resource is referenced it's because it was a permanent resource and some other resource
                // had a dependency on it, since we moved all permanent resource into the m_ResourcesPendingDeletion
                // vector its owner (the parent resource) is also inside this vector. We should ignore the resource
                // for now until it's parent is deleted and it is finally possible to delete it
                if (resource->IsReferenced())
                {
                    continue;
                }

                // Set pending deletion for this resource
                resource->SetPendingDeletion();

                // Add this object into the deletion queue, takes ownership
                m_ResourceDeleter.DeleteObject(std::move(resource), resource->GetFactoryPtr());

                // Set that we deleted some resources
                resourceDeletionIsStable = false;

                // Remove the resource from this queue
                m_ResourcesPendingDeletion.erase(m_ResourcesPendingDeletion.begin() + i);
            }
        }

        // Keep looping until there are no more deletions available
    } while (!resourceDeletionIsStable);

    assert(m_ModifiedFiles.size_approx() == 0);
    assert(m_ResourceCreateProxyQueue.size_approx() == 0);
    assert(m_ResourcesPendingExternalConstruction.size_approx() == 0);
    assert(m_ResourcesPendingDeletionEvaluation.size_approx() == 0);
    assert(m_ResourcesPendingDeletion.size() == 0);
    assert(m_ResourcesPendingReplacement.size() == 0);

    m_RegisteredFactories.clear();
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

uint32_t PacketResourceManager::GetAproximatedResourceAmount() const
{
    return m_ResourceStorage.GetAproximatedResourceAmount() +
        GetApproximatedNumberResourcesPendingDeletion();
}


void PacketResourceManager::RegisterResourceForDeletion(PacketResource * _resourcePtr)
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

uint32_t PacketResourceManager::GetApproximatedNumberResourcesPendingDeletion() const
{
    return static_cast<uint32_t>(m_ResourcesPendingDeletion.size());
}

void PacketResourceManager::AsynchronousResourceProcessment()
{
    // For each file that was modified, call the OnResourceDataChanged method for its resource, 
    // if any
    {
        HashPrimitive resource_hash;
        while (m_ModifiedFiles.try_dequeue(resource_hash))
        {
            // Retrieve all resource associated with this hash
            auto associated_resources = m_ResourceStorage.GetAllObjectsWithHash(resource_hash);
            for (auto* resource : associated_resources)
            {
                OnResourceDataChanged(resource);
            }
        }  
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
        auto& [creationProxy, factory, buildInfo, hash, isPermanent, resourceData] = resourceCreationData;

        // Check if we need to create and load this resource
        PacketResource* resource = m_ResourceStorage.FindObject(hash, buildInfo.buildFlags);
        if (resource == nullptr)
        {
            // Load this resource
            auto resourceUniquePtr = m_ResourceLoader.LoadObject(
                factory,
                hash,
                buildInfo,
                isPermanent,
                std::move(resourceData));
            resource = resourceUniquePtr.get();
            assert(resource != nullptr);

            // Watch this resource file object (not enabled on release and non-edit builds)
            // m_ResourceWatcher.WatchResource(resource);

            // Register this resource inside the storage
            m_ResourceStorage.InsertObject(std::move(resourceUniquePtr), hash, buildInfo.buildFlags);

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
            // Register the replacing resource
            originalResource->RegisterReplacingResource(newResource.get());

            // Replace the new resource inside the storage (takes ownership)
            std::unique_ptr<PacketResource> oldResource = m_ResourceStorage.ReplaceObject(
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
        std::unique_ptr<PacketResource> objectUniquePtr = m_ResourceStorage.GetObjectOwnership(resourcePtr);
        if (objectUniquePtr == nullptr)
        {
            // This resource was already set to be deleted, there are expected occasions (like the one mentioned
            // on the IsReferenced() check above) that will make a resource be released multiple times
            continue;
        }

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

void PacketResourceManager::RegisterResourceForExternalConstruction(PacketResource * _resource)
{
    m_ResourcesPendingExternalConstruction.enqueue(_resource);
}

void PacketResourceManager::OnResourceDataChanged(PacketResource* _resource)
{
    // Disabled on non edit builds
    if (m_OperationMode != OperationMode::Plain)
    {
        return;
    }

    // Check if this resource is pending deletion
    if (_resource->IsPendingDeletion() || !_resource->IsReferenced())
    {
        // We can't proceed with this resource on its current status
        m_LoggerPtr->LogWarning(
            std::string("Found file modification event on file: \"")
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
            m_LoggerPtr->LogWarning(
                std::string("Found duplicated file modification event on file: \"")
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
            m_LoggerPtr->LogWarning(
                std::string("Found file modification event on file: \"")
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
        m_LoggerPtr->LogWarning(
            std::string("Found file modification event on file: \"")
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
    auto resourceUniquePtr = m_ResourceLoader.LoadObject(
        _resource->GetFactoryPtr(),
        _resource->GetHash(),
        _resource->GetBuildInfo(),
        _resource->IsPermanent(),
        {});

    // Increment the number of references of this resource, since it will be replacing the old resource
    // when this very resource is deleted it will remove this added reference
    resourceUniquePtr->IncrementNumberReferences();

    // If this resource requires external construct, enqueue it on the correspondent queue
    if (resourceUniquePtr->RequiresExternalConstructPhase() && !resourceUniquePtr->ConstructionFailed())
    {
        m_ResourcesPendingExternalConstruction.enqueue(resourceUniquePtr.get());
    }
    else if (!resourceUniquePtr->ConstructionFailed())
    {
        // If this resource doesn't need external construct, call it here to set the internal flags
        resourceUniquePtr->BeginExternalConstruct(nullptr);
    }

    // Move the resource into the replace queue
    m_ResourcesPendingReplacement.push_back({ std::move(resourceUniquePtr), _resource });
}