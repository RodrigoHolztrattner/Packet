////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceManager.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResourceManager.h"
#include "PacketResourceInstance.h"
#include "PacketResourceFactory.h"
#include "PacketResourceWatcher.h"

#include <cassert>

#define ThreadSleepTimeMS 3

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

    // Move all instances that are pending release valuation to the release vector
    {
        std::unique_ptr<PacketResourceInstance> instanceReleaseObject;
        while (m_InstancesPendingReleaseEvaluation.try_dequeue(instanceReleaseObject))
        {
            // Just move it to the pending release vector
            m_InstancesPendingRelease.push_back(std::move(instanceReleaseObject));
        }
    }

    // Instanced pending evaluation
    {
        // Sorry but no instance will be evaluated to receive its resource
        InstanceEvaluationData instanceEvaluationData;
        while (m_InstancesPendingEvaluation.try_dequeue(instanceEvaluationData))
        {
            // Get the info
            auto[instance, factory, buildInfo, hash, isPermanent, isRuntime, resourceData] = instanceEvaluationData;

            // If we have an instance here this should mean that a resource was requested but right after the program was
            // set to shutdown, it's expected that the packet system will be destroyed after all instances were released
            // so we must find this same instance on the release vector, if this isn't true the packet system is being
            // destroyed with active instances -> A bad logic design.
            auto FindInstance = [&](PacketResourceInstance* _instance)
            {
                for (auto& instanceUniquePtr : m_InstancesPendingRelease)
                {
                    if (instanceUniquePtr.get() == _instance)
                    {
                        return true;
                    }
                }

                return false;
            };
            assert(FindInstance(instance) && "Packet resource system is being deleted but some resource instances \
are still active, this will probably lead into exceptions!");
            
            // There is no right way of dealing with instances that aren't being destroyed but requested evaluation,
            // I cannot just make it be released because it can be in used and that would generate problems, also
            // leaving it and destroying the packet system will probably result in errors too since it potentially
            // could try to access objects that aren't valid anymore.
            // Also I cannot throw here, the ideal would be throwing an exception.
        }
    }

    // Instanced pending construction
    {
        // Sorry but no instance will be constructed, also no need to release them here since these
        // pointers are weak references, they will be released by methods below
        m_InstancesPendingConstruction.clear();
    }

    // Instanced pending release
    {
        // Ok we can release these instances, but their resources are going to be deleted manually
        for (auto& instance : m_InstancesPendingRelease)
        {
            // Remove the instance reference from its resource
            PacketResource* resource = instance->GetResource();
            PacketResourceFactory* factory = instance->GetFactoryPtr();

            // If we requested this instance and it was released right after the request, there is a 
            // possibility that its resource doesn't exist
            if (resource != nullptr)
            {
                // Remove the reference originally from this instance
                resource->RemoveInstanceReference(instance.get());

                // Check if the resource should be deleted
                if (!resource->IsDirectlyReferenced() && !resource->IsPermanent())
                {
                    // Remove this object from the storage, taking its ownership back
                    std::unique_ptr<PacketResource> objectUniquePtr = m_ResourceStoragePtr->GetObjectOwnership(resource);

                    // Insert this object into the deletion queue
                    m_ResourcesPendingDeletion.push_back(std::move(objectUniquePtr));
                }
            }

            // Delete the instance using its factory object
            factory->ReleaseInstance(std::move(instance));     
        }

        m_InstancesPendingRelease.clear();
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
            originalResourceUniquePtr->DecrementNumberDirectReferences();
            assert(!originalResourceUniquePtr->IsReferenced());
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

    assert(m_InstancesPendingEvaluation.size_approx()             == 0);
    assert(m_InstancesPendingReleaseEvaluation.size_approx()      == 0);
    assert(m_ResourcesPendingExternalConstruction.size_approx()   == 0);
    assert(m_ResourcesPendingModificationEvaluation.size_approx() == 0);
    assert(m_InstancesPendingConstruction.size()                  == 0);
    assert(m_InstancesPendingRelease.size()                       == 0);
    assert(m_ResourcesPendingDeletion.size()                      == 0);
    assert(m_ResourcesPendingReplacement.size()                   == 0);
}

bool PacketResourceManager::WaitForInstance(const PacketResourceInstance* _instance,
                                            long long _timeout) const
{
    // The instance must be valid
    if (_instance == nullptr)
    {
        return false;
    }

    clock_t initialTime = clock();
    clock_t currentTime = initialTime;
    while (true)
    {
        if (_timeout != -1 &&
            double(currentTime - initialTime) / CLOCKS_PER_SEC >= double(_timeout) / 1000)
        {
            return false;
        }

        if (_instance->IsReady())
        {
            return true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(ThreadSleepTimeMS));

        currentTime = clock();
    }

    return false;
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

void PacketResourceManager::ReleaseInstanceOnUnlink(std::unique_ptr<PacketResourceInstance> _instancePtr)
{
	// Push the new release request, no need to synchronize this because we use a lock-free queue
    m_InstancesPendingReleaseEvaluation.enqueue(std::move(_instancePtr));
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

            // Construct the resource
            resource->OnConstruct();

            // If this resource requires external construct, enqueue it on the correspondent queue
            if (resource->RequiresExternalConstructPhase() && !resource->ConstructionFailed())
            {
                m_ResourcesPendingExternalConstruction.enqueue(resource);
            }
        }

        // Make the instance reference it
        resource->MakeInstanceReference(instance);

        // Insert this instance into the wait vector
        m_InstancesPendingConstruction.push_back(instance);
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
        // Check if the new resource failed to be constructed
        else if (newResource->ConstructionFailed())
        {
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

    // For each instance pending modification
    for (int i = static_cast<int>(m_ResourcesPendingModification.size() - 1); i >= 0; i--)
    {
        PacketResource* resource = m_ResourcesPendingModification[i];

        // Verify is this resource doesn't have indirect references
        if (resource->IsIndirectlyReferenced())
        {
            continue;
        }

        // Begin the modifications
        resource->BeginModifications();

        m_ResourcesPendingModification.erase(m_ResourcesPendingModification.begin() + i);
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

        // Check if this resource is referenced in any way and if it is pending modifications
        if (resource->IsReferenced() || resource->IsPendingModifications())
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
    for (int i = static_cast<int>(m_InstancesPendingConstruction.size() - 1); i >= 0; i--)
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