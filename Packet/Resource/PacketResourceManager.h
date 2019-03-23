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
#include "PacketResourceFactory.h"

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
		PacketLogger* _loggerPtr);
	~PacketResourceManager();
	
//////////////////
// MAIN METHODS //
protected: ///////

    template <typename ResourceInstance>
    bool RequestResource(PacketResourceInstancePtr<ResourceInstance>& _instancePtr,
                         PacketResourceFactory* _factoryPtr, 
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
        std::unique_ptr<PacketResourceInstance> newInstance = _factoryPtr->RequestInstance(_hash, this);

        // Link the new instance with the instance ptr, takes ownership from the unique_ptr
        _instancePtr.InstanceLink(std::move(newInstance));

        // Add this new instance to our evaluation queue
        m_InstancesPendingEvaluation.enqueue({ _instancePtr.Get(), 
                                             _factoryPtr, 
                                             _resourceBuildInfo,
                                             _hash,
                                             _isPersistent,
                                             false, 
                                             {} });

        return true;
    }

    template <typename ResourceInstance>
    void RequestRuntimeResource(PacketResourceInstancePtr<ResourceInstance>& _instancePtr,
                                PacketResourceFactory* _factoryPtr,
                                PacketResourceBuildInfo _resourceBuildInfo,
                                std::vector<uint8_t> _resourceData = {})
    {
        // Create a new resource instance object
        std::unique_ptr<PacketResourceInstance> newInstance = _factoryPtr->RequestInstance(Hash(), this);

        // Link the new instance with the instance ptr, takes ownership from the unique_ptr
        _instancePtr.InstanceLink(std::move(newInstance));

        // Add this new instance to our evaluation queue
        m_InstancesPendingEvaluation.enqueue({ _instancePtr.Get(),
                                             _factoryPtr, 
                                             _resourceBuildInfo,
                                             Hash(), 
                                             false, 
                                             true, 
                                             std::move(_resourceData)});
    }
  
    // The asynchronous resource process method
    void AsynchronousResourceProcessment();

protected:

	// This method is called when a resource file is modified, its execution will happen when inside the 
	// update phase. This method won't be called when on release or non edit builds
	void OnResourceDataChanged(PacketResource* _resource);

	// Release an object instance, this method must be called only by a packet resource instance ptr when it is deleted without
	// being moved to another variable using move semantics
	void ReleaseObject(std::unique_ptr<PacketResourceInstance> _instance);

	// This method must be called by a instance object that needs to be reconstructed, this method must be
	// called when doing the update phase here on this class (doesn't need to be called necessarilly inside
	// this object but we must ensure we are doing an update)
	void ReconstructInstance(PacketResourceInstance* _instance);

///////////////
// VARIABLES //
private: //////

    typedef std::tuple<
        PacketResourceInstance*,
        PacketResourceFactory*,
        PacketResourceBuildInfo,
        Hash,
        bool,
        bool,
        std::vector<uint8_t>>
        InstanceEvaluationData;
    
    // Instance queues/vectors
    moodycamel::ConcurrentQueue<InstanceEvaluationData>                  m_InstancesPendingEvaluation;
    std::vector<PacketResourceInstance*>                                 m_InstancesPendingConstruction;
    moodycamel::ConcurrentQueue<std::unique_ptr<PacketResourceInstance>> m_InstancesPendingReleaseEvaluation;
    std::vector<std::unique_ptr<PacketResourceInstance>>                 m_InstancesPendingRelease;

    // Resource queues/vector
    moodycamel::ConcurrentQueue<PacketResource*> m_ResourcesPendingExternalConstruction;
    moodycamel::ConcurrentQueue<PacketResource*> m_ResourcesPendingPostConstruction;
    std::vector<std::unique_ptr<PacketResource>> m_ResourcesPendingDeletion;
    std::vector<std::pair<std::unique_ptr<PacketResource>, PacketResource*>> m_ResourcesPendingReplacement;

    

    // The asynchronous management thread and the conditional to exit
    std::thread m_AsynchronousManagementThread;
    bool m_AsynchronousManagementThreadShouldExit = false;

	// The resource loader and deleter
	PacketResourceLoader m_ResourceLoader;
	PacketResourceDeleter m_ResourceDeleter;

	// The object requests and the release queue

	// The construct, deletion and replace queues
	

	// The current operation mode
	OperationMode m_OperationMode;

	// The object storage, the file loader, the resource watcher, the reference manager and the logger ptrs
	PacketResourceStorage*  m_ResourceStoragePtr;
	PacketResourceWatcher*  m_ResourceWatcherPtr;
	PacketFileLoader*       m_FileLoaderPtr;
	PacketReferenceManager* m_ReferenceManagerPtr;
	PacketLogger*           m_Logger;
};

// Packet
PacketDevelopmentNamespaceEnd(Packet)