////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceManager.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "..\PacketConfig.h"
#include "PacketResource.h"
#include "PacketMultipleQueue.h"
#include "PacketResourceLoader.h"
#include "PacketResourceDeleter.h"
#include "PacketResourceStorage.h"

#include "concurrentqueue.h"

#include <cstdint>
#include <vector>
#include <thread>

///////////////
// NAMESPACE //
///////////////

/////////////
// DEFINES //
/////////////

////////////
// GLOBAL //
////////////

///////////////
// NAMESPACE //
///////////////

// Packet
PacketDevelopmentNamespaceBegin(Packet)

//////////////
// TYPEDEFS //
//////////////

////////////////
// FORWARDING //
////////////////

// Classes we know
class PacketResourceInstance;
class PacketReferenceManager;
class PacketResourceFactory;
class PacketResourceWatcher;
class PacketSystem;
template <typename InstanceClass>
struct PacketResourceInstancePtr;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketResourceManager
////////////////////////////////////////////////////////////////////////////////
class PacketResourceManager
{
public:

	// Friend classes
	friend PacketResourceInstance;
	friend PacketSystem;

	// The request type
	struct ObjectRequest
	{
		// The instance and hash variables
		PacketResourceInstance* instance;
		Hash hash;

		// The build info
		PacketResourceBuildInfo buildInfo;

		// The factory ptr
		PacketResourceFactory* factoryPtr;

		// If the object is permanent
		bool isPermanent;
	};

	// The release type
	struct ObjectRelease
	{
		// The instance
		std::unique_ptr<PacketResourceInstance> instance;

		// The factory ptr
		PacketResourceFactory* factoryPtr;

		// If this object should be deleted synchronous
		bool deleteSync;
	};

	// The deletion type
	struct DeletionRequest
	{
		// The resource
		std::unique_ptr<PacketResource> resource;

		// If this object should be deleted synchronous
		bool deleteSync;
	};

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketResourceManager(OperationMode _operationMode,
		PacketResourceStorage* _storagePtr,
		PacketFileLoader* _fileLoaderPtr, 
		PacketReferenceManager* _referenceManager, 
		PacketResourceWatcher* _resourceWatcherPtr, 
		PacketLogger* _loggerPtr, 
		uint32_t _workerThreads, 
		ThreadIndexRetrieveMethod _threadIndexMethod);
	~PacketResourceManager();
	
//////////////////
// MAIN METHODS //
protected: ///////

    template <typename ResourceInstance>
    bool RequestResource(PacketResourceInstancePtr<ResourceInstance>& _instancePtr,
                         Hash _hash, 
                         bool _isPersistent,
                         PacketResourceBuildInfo _resourceBuildInfo)
    {
        // Check if a resource with the given hash exist
        if (!m_FileLoaderPtr->FileExist(_hash))
        {
            // The file doesn't exist
            m_Logger->LogError(std::string("Trying to load file at path: \"")
                               .append(_hash.GetPath())
                               .append("\" and hash: ")
                               .append(std::to_string(_hash.GetHashValue()))
                               .append(" but it doesn't exist on our database!")
                               .c_str());

            return false;
        }

        // Create a new resource instance object
        std::unique_ptr<PacketResourceInstance> newInstance = std::make_unique<PacketResourceInstance>(_hash, this);

        // Link the new instance with the instance ptr, takes ownership from the unique_ptr
        _instancePtr.InstanceLink(std::move(newInstance));

        // Add this new instance to our evaluation queue
        m_InstancesPendingEvaluation.enqueue({ _instancePtr.Get(), _resourceBuildInfo, _hash, _isPersistent });
    }

    // Request an object for the given instance and resource hash
	template <typename ResourceInstance, typename ResourceFactory>
	bool RequestResource(PacketResourceInstancePtr<ResourceInstance>& _instancePtr, Hash _hash, ResourceFactory* _factoryPtr, bool _isPersistent, PacketResourceBuildInfo _resourceBuildInfo)
	{
		// Asserts
		assert(!m_InUpdatePhase || (m_InUpdatePhase && std::this_thread::get_id() == m_UpdateThreadID));

		// Check if a resource with the given hash exist
		if (!m_FileLoaderPtr->FileExist(_hash))
		{
			// The file doesn't exist
			m_Logger->LogError(std::string("Trying to load file at path: \"")
				.append(_hash.GetPath())
				.append("\" and hash: ")
				.append(std::to_string(_hash.GetHashValue()))
				.append(" but it doesn't exist on our database!")
				.c_str());

			return false;
		}

		// Create a new resource instance object
		std::unique_ptr<PacketResourceInstance> newInstance = _factoryPtr->RequestInstance(_hash, this);

		// Get a pointer to this new instance because we will move it into the instance ptr
		PacketResourceInstance* newInstancePtr = newInstance.get();

		// Link the new instance with the instance ptr, takes ownership from the unique_ptr
		_instancePtr.InstanceLink(newInstance);

		// Check if we already have an object with this hash, if the object was loaded and if we can construct this instance asynchronous
		PacketResource* object = m_ResourceStoragePtr->FindObject(_hash, _resourceBuildInfo.buildFlags);
		if (object != nullptr && object->IsReady() && _resourceBuildInfo.asyncInstanceConstruct)
		{
			// In case we are requesting a pesistent object, check if the already loaded object was created using the permanent mode
			assert(!_isPersistent || (_isPersistent && object->IsPersistent()) && "Trying to request a permanent object but it was already loaded and is not permanent!");

			// Make the instance reference it
			object->MakeInstanceReference(newInstancePtr);

			// Construct this instance
			newInstancePtr->BeginConstruction();
		}
		else
		{
			// Create the new request
			ObjectRequest request = { newInstancePtr, _hash, _resourceBuildInfo, _factoryPtr, _isPersistent };

			// Push the new request
			m_ResourceRequests.Insert(request);
		}

		return true;
	}

	// The update method, process all requests
	void Update();

    // The asynchronous resource process method
    void AsynchronousResourceProcessment();

protected:

	// This method is called when a resource file is modified, its execution will happen when inside the 
	// update phase. This method won't be called when on release or non edit builds
	void OnResourceDataChanged(PacketResource* _resource);

	// Release an object instance, this method must be called only by a packet resource instance ptr when it is deleted without
	// being moved to another variable using move semantics
	void ReleaseObject(std::unique_ptr<PacketResourceInstance>& _instance, PacketResourceFactory* _factoryPtr, bool _allowAsynchronousDeletion = false);

	// This method must be called by a instance object that needs to be reconstructed, this method must be
	// called when doing the update phase here on this class (doesn't need to be called necessarilly inside
	// this object but we must ensure we are doing an update)
	void ReconstructInstance(PacketResourceInstance* _instance);

///////////////
// VARIABLES //
private: //////
    
    // Our concurrent queues
    moodycamel::ConcurrentQueue<std::tuple<PacketResourceInstance*, PacketResourceBuildInfo, Hash, bool>> m_InstancesPendingEvaluation;
    moodycamel::ConcurrentQueue<PacketResource*> m_ResourcesPendingSynchronousConstruction;
    moodycamel::ConcurrentQueue<PacketResource*> m_ResourcesPendingAsynchronousConstruction;
    moodycamel::ConcurrentQueue<PacketResource*> m_ResourcesPendingExternalConstruction;

    std::vector<PacketResourceInstance*> m_InstancesWaitingForResourceConstruction;


    moodycamel::ConcurrentQueue<PacketResourceInstance*> m_InstancesPendingRelease;
    moodycamel::ConcurrentQueue<PacketResourceInstance*> m_ResourcesPendingDeletion;

    // Our mutexes
    std::mutex m_StorageMutex;
	std::mutex m_Mutex;

	// The resource loader and deleter
	PacketResourceLoader m_ResourceLoader;
	PacketResourceDeleter m_ResourceDeleter;

	// The object requests and the release queue
	MultipleQueue<ObjectRequest> m_ResourceRequests;
	MultipleQueue<ObjectRelease> m_InstanceReleases;

	// The construct, deletion and replace queues
	std::vector<PacketResourceInstance*> m_ConstructQueue;
	std::vector<DeletionRequest> m_DeletionQueue;
	std::vector<PacketResource*> m_ReplaceQueue;

	// The current operation mode
	OperationMode m_OperationMode;

	// The object storage, the file loader, the resoruce watcher, the reference manager and the logger ptrs
	PacketResourceStorage* m_ResourceStoragePtr;
	PacketResourceWatcher* m_ResourceWatcherPtr;
	PacketFileLoader* m_FileLoaderPtr;
	PacketReferenceManager* m_ReferenceManagerPtr;
	PacketLogger* m_Logger;

	// If we are inside the update phase and the updating thread id (used for asserts only)
	bool m_InUpdatePhase;
	std::thread::id m_UpdateThreadID;
};

// Packet
PacketDevelopmentNamespaceEnd(Packet)