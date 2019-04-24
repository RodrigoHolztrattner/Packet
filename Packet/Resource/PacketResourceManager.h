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

#define ThreadSleepTimeMS 3

////////////////
// FORWARDING //
////////////////

// Classes we know
class PacketResourceInstance;
class PacketResource;
class PacketResourceExternalConstructor;
class PacketReferenceManager;
class PacketResourceFactory;
class PacketResourceWatcher;
class PacketSystem;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketResourceManager
////////////////////////////////////////////////////////////////////////////////
class PacketResourceManager
{
public:

	// Friend classes
	friend PacketResourceInstance;
    friend PacketResource;
    friend PacketResourceExternalConstructor;
	friend PacketSystem;

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

    template <typename ResourceClass>
    bool RequestResource(PacketResourceReference<ResourceClass>& _resourceReference,
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

        // Get a proxy for this resource reference
        auto resourceCreationProxy = GetResourceCreationProxy();

        // Register the creation proxy
        _resourceReference.RegisterCreationProxy(resourceCreationProxy.get());

        // Add this new creation data into our processing queue
        m_ResourceCreateProxyQueue.enqueue({ std::move(resourceCreationProxy),
                                             _factoryPtr, 
                                             _resourceBuildInfo,
                                             _hash,
                                             _isPersistent,
                                             false, 
                                             std::vector<uint8_t>() });

        return true;
    }

    template <typename ResourceClass>
    void RequestRuntimeResource(PacketResourceReference<ResourceClass>& _resourceReference,
                                PacketResourceFactory* _factoryPtr,
                                PacketResourceBuildInfo _resourceBuildInfo = PacketResourceBuildInfo(),
                                std::vector<uint8_t> _resourceData = {})
    {
        // Get a proxy for this resource reference
        auto resourceCreationProxy = GetResourceCreationProxy();

        // Register the creation proxy
        _resourceReference.RegisterCreationProxy(resourceCreationProxy.get());

        // Add this new creation data into our processing queue
        m_ResourceCreateProxyQueue.enqueue({ std::move(resourceCreationProxy),
                                             _factoryPtr, 
                                             _resourceBuildInfo,
                                             Hash(), 
                                             false, 
                                             true, 
                                             std::move(_resourceData)});
    }
  
    // This method will wait until the given resource is ready to be used
    // Optionally you can pass a timeout parameter in milliseconds
    template <typename ResourceClass>
    bool WaitForResource(const PacketResourceReference<ResourceClass>& _resourceReference,
                         long long _timeout = -1) const
    {
        clock_t initialTime = clock();
        clock_t currentTime = initialTime;
        while (true)
        {
            if (_timeout != -1 &&
                double(currentTime - initialTime) / CLOCKS_PER_SEC >= double(_timeout) / 1000)
            {
                return false;
            }

            if (_resourceReference)
            {
                return true;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(ThreadSleepTimeMS));

            currentTime = clock();
        }

        return false;
    }

    // This method will return a vector of resource external constructor object that must be constructed
    // by the user externally (since the resource requires it)
    std::vector<PacketResourceExternalConstructor> GetResourceExternalConstructors();

protected:

    // Return a valid usable resource creation proxy object
    std::unique_ptr<PacketResourceCreationProxy> GetResourceCreationProxy();

    // This method return the approximated number of resources that are pending deletion
    uint32_t GetApproximatedNumberResourcesPendingDeletion();

    // The asynchronous resource process method
    void AsynchronousResourceProcessment();

	// This method is called when a resource file is modified, its execution will happen when inside the 
	// update phase. This method won't be called when on release or non edit builds
	void OnResourceDataChanged(PacketResource* _resource);

	// Called directly by a resource when its reference count reaches zero, this will insert the resource
    // into the deletion queue
	void RegisterResourceForDeletion(PacketResource* _resourcePtr);

    // This method will register a resource to be modified after it reaches 0 indirect references
    // Only the resource should call this method
    void RegisterResourceForModifications(PacketResource* _resource);

    // This method will register a resource to be externally constructed, it should be called from a 
    // PacketResourceExternalConstructor object if it goes out of scope without constructing the 
    // underlying resource. By calling this we will make sure the resource has the change to be 
    // picked for construction again
    void RegisterResourceForExternalConstruction(PacketResource* _resource);

///////////////
// VARIABLES //
private: //////

    typedef std::tuple<
        std::unique_ptr<PacketResourceCreationProxy>,
        PacketResourceFactory*,
        PacketResourceBuildInfo,
        Hash,
        bool,
        bool,
        std::vector<uint8_t>>
        ResourceCreationData;

    // Proxy queues
    moodycamel::ConcurrentQueue<ResourceCreationData>                         m_ResourceCreateProxyQueue;
    moodycamel::ConcurrentQueue<std::unique_ptr<PacketResourceCreationProxy>> m_ResourceCreateProxyFreeQueue;
   
    // Resource queues/vector
    moodycamel::ConcurrentQueue<PacketResource*>                             m_ResourcesPendingExternalConstruction;
    moodycamel::ConcurrentQueue<PacketResource*>                             m_ResourcesPendingDeletionEvaluation;
    std::vector<std::unique_ptr<PacketResource>>                             m_ResourcesPendingDeletion;
    std::vector<std::pair<std::unique_ptr<PacketResource>, PacketResource*>> m_ResourcesPendingReplacement;
    moodycamel::ConcurrentQueue<PacketResource*>                             m_ResourcesPendingModificationEvaluation;
    std::vector<PacketResource*>                                             m_ResourcesPendingModification;

    // The asynchronous management thread and the conditional to exit
    std::thread m_AsynchronousManagementThread;
    bool        m_AsynchronousManagementThreadShouldExit = false;

	// The resource loader and deleter
	PacketResourceLoader  m_ResourceLoader;
	PacketResourceDeleter m_ResourceDeleter;

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