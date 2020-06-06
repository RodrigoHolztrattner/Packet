////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceManager.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "../PacketConfig.h"
#include "../File/Loader/PacketFileLoader.h"
#include "../File/Indexer/PacketFileIndexer.h"
#include "PacketResource.h"
#include "../PacketMultipleQueue.h"
#include "Loader/PacketResourceLoader.h"
#include "Deleter/PacketResourceDeleter.h"
#include "Storage/PacketResourceStorage.h"
#include "Factory/PacketResourceFactory.h"

#include "concurrentqueue.h"

#include <cstdint>
#include <vector>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

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
class PacketResourceFactory;
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
	PacketResourceManager(
        OperationMode            _operationMode,
		PacketResourceStorage&   _storagePtr,
		const PacketFileLoader&  _fileLoaderPtr, 
        PacketFileIndexer&       _fileIndexer,
		PacketLogger*            _loggerPtr);
	~PacketResourceManager();
	
//////////////////
// MAIN METHODS //
public: //////////

    // Register a resource factory
    template <typename ResourceFactoryClass, typename ResourceClass, typename ... Args>
    void RegisterResourceFactory(Args&& ... args)
    {
        assert(m_RegisteredFactories.find(ctti::type_id<ResourceClass>().hash()) == m_RegisteredFactories.end());
        m_RegisteredFactories.insert({ ctti::type_id<ResourceClass>().hash(),
                                     std::make_unique<ResourceFactoryClass>(std::forward<Args>(args) ...) });
    }

    // Request an object for the given resource reference and hash
    template <typename ResourceClass>
    bool RequestResource(PacketResourceReference<ResourceClass>& _resourceReference,
                         Hash _hash,
                         PacketResourceBuildInfo _resourceBuildInfo = PacketResourceBuildInfo())
    {
        assert(m_RegisteredFactories.find(ctti::type_id<ResourceClass>().hash()) != m_RegisteredFactories.end());

        // Shared lock that allows concurrent access but will lock in case we need to stop all operations
        // by deep locking this mutex
        std::optional<std::shared_lock<std::shared_mutex>> lock;
        if (m_OperationMode == OperationMode::Plain)
        {
            lock = std::shared_lock(m_RequestMutex);
        }
        
        // Check if a resource with the given hash exist
        if (!m_FileIndexer.IsFileIndexed(_hash))
        {
            // The file doesn't exist
            m_LoggerPtr->LogError(
                std::string("Trying to load file at path: \"")
                .append(_hash.path().string())
                .append("\" and hash: ")
                .append(std::to_string(_hash.get_hash_value()))
                .append(" but it doesn't exist on our database!")
                .c_str());

            return false;
        }

        // Get a proxy for this resource reference
        auto resourceCreationProxy = GetResourceCreationProxy();

        _resourceReference.RegisterCreationProxy(resourceCreationProxy.get());
        _resourceReference.RegisterResourceHash(_hash);

        // Add this new creation data into our processing queue
        m_ResourceCreateProxyQueue.enqueue({ std::move(resourceCreationProxy),
                                             m_RegisteredFactories[ctti::type_id<ResourceClass>().hash()].get(),
                                             _resourceBuildInfo,
                                             _hash,
                                             false,
                                             std::vector<uint8_t>() });

        return true;
    }

    // Request a permanent object for the given reference and resource hash, the object will not be deleted when it reaches 0
    // references, the deletion phase will only occur in conjunction with the storage deletion
    template <typename ResourceClass>
    bool RequestPermanentResource(PacketResourceReference<ResourceClass> & _resourceReference,
                                  Hash _hash,
                                  PacketResourceBuildInfo _resourceBuildInfo = PacketResourceBuildInfo())
    {
        assert(m_RegisteredFactories.find(ctti::type_id<ResourceClass>().hash()) != m_RegisteredFactories.end());

        // Shared lock that allows concurrent access but will lock in case we need to stop all operations
        // by deep locking this mutex
        std::optional<std::shared_lock<std::shared_mutex>> lock;
        if (m_OperationMode == OperationMode::Plain)
        {
            lock = std::shared_lock(m_RequestMutex);
        }

        // Check if a resource with the given hash exist
        if (!m_FileIndexer.IsFileIndexed(_hash))
        {
            // The file doesn't exist
            m_LoggerPtr->LogError(
                std::string("Trying to load file at path: \"")
                .append(_hash.path().string())
                .append("\" and hash: ")
                .append(std::to_string(_hash.get_hash_value()))
                .append(" but it doesn't exist on our database!")
                .c_str());

            return false;
        }

        // Get a proxy for this resource reference
        auto resourceCreationProxy = GetResourceCreationProxy();

        _resourceReference.RegisterCreationProxy(resourceCreationProxy.get());
        _resourceReference.RegisterResourceHash(_hash);

        // Add this new creation data into our processing queue
        m_ResourceCreateProxyQueue.enqueue({ std::move(resourceCreationProxy),
                                             m_RegisteredFactories[ctti::type_id<ResourceClass>().hash()].get(),
                                             _resourceBuildInfo,
                                             _hash,
                                             true,
                                             std::vector<uint8_t>() });

        return true;
    }

    // This method will wait until the given resource is ready to be used
    // Optionally you can pass a timeout parameter in milliseconds
    template <typename ResourceClass>
    bool WaitForResource(
        const PacketResourceReference<ResourceClass>& _resourceReference,
        long long                                     _timeout = -1) const
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

    // Return an approximation of the current number of resources since some of them could be enqueued 
    // to be created or released
    uint32_t GetAproximatedResourceAmount() const;

    // Lock all operations for this resource manager, returning two unique_lock objects that when destroyed,
    // will resume all operations, this function is only available on Plain mode and will throw an exception
    // in other modes
    std::pair<std::unique_lock<std::shared_mutex>, std::unique_lock<std::mutex>> LockAllOperations();

protected:

    // Return a valid usable resource creation proxy object
    std::unique_ptr<PacketResourceCreationProxy> GetResourceCreationProxy();

    // This method return the approximated number of resources that are pending deletion
    uint32_t GetApproximatedNumberResourcesPendingDeletion() const;

    // The asynchronous resource process method
    void AsynchronousResourceProcessment();

	// This method is called when a resource file is modified, its execution will happen when inside the 
	// update phase. This method won't be called when on release or non edit builds
	void OnResourceDataChanged(PacketResource* _resource);

	// Called directly by a resource when its reference count reaches zero, this will insert the resource
    // into the deletion queue
	void RegisterResourceForDeletion(PacketResource* _resourcePtr);

    // This method will register a resource to be externally constructed, it should be called from a 
    // PacketResourceExternalConstructor object if it goes out of scope without constructing the 
    // underlying resource. By calling this we will make sure the resource has the change to be 
    // picked for construction again
    void RegisterResourceForExternalConstruction(PacketResource* _resource);

///////////////
// VARIABLES //
private: //////

    // All registered resource factories
    std::unordered_map<uint64_t, std::unique_ptr<PacketResourceFactory>> m_RegisteredFactories;

    typedef std::tuple<
        std::unique_ptr<PacketResourceCreationProxy>,
        PacketResourceFactory*,
        PacketResourceBuildInfo,
        Hash,
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

    // File modification queue
    moodycamel::ConcurrentQueue<HashPrimitive>                               m_ModifiedFiles;

    // The asynchronous management thread and the conditional to exit
    std::thread m_AsynchronousManagementThread;
    bool        m_AsynchronousManagementThreadShouldExit = false;

	// The resource loader and deleter
	PacketResourceLoader  m_ResourceLoader;
	PacketResourceDeleter m_ResourceDeleter;

	// The current operation mode
	OperationMode m_OperationMode;

    // These mutexes will be used to enforce an idle status when required while also enabling the total lock
    // of resource operations if necessary, only available on Plain mode
    mutable std::shared_mutex m_RequestMutex;
    mutable std::mutex        m_ProcessMutex;
    mutable std::mutex        m_ModificationMutex;

	// The object storage, the file loader, the resource watcher, the reference manager and the logger ptrs
	PacketResourceStorage&  m_ResourceStorage;
    const PacketFileLoader& m_FileLoader;
    PacketFileIndexer&      m_FileIndexer;
	PacketLogger*           m_LoggerPtr;
};

// Packet
PacketDevelopmentNamespaceEnd(Packet)