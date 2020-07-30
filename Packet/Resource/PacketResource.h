////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResource.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "../PacketConfig.h"

#include <memory>
#include <cstdint>
#include <vector>
#include <atomic>
#include <mutex>
#include <optional>

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
class PacketFileLoader;
class PacketResourceInstance;
class PacketResourceManager;
class PacketResourceLoader;
class PacketResourceDeleter;
class PacketResourceFactory;
class PacketResourceStorage;
class PacketReferenceManager;
class PacketResourceExternalConstructor;
class PacketResourceCreationProxy;
class PacketResourceCreationProxyInterface;
class PacketResourceKeepAlive;

template <typename ResourceClass>
class PacketResourceReference;

// The structure that will be used to store the resource data, it works like a vector of uint8_t and is allocation 
// and deallocation must be managed by a factory class
struct PacketResourceData
{
    // Friend classes
    friend PacketResourceLoader;

	// Our constructors
	PacketResourceData();
    PacketResourceData(std::vector<uint8_t> _data);

	// Return the data
	const std::vector<uint8_t>& GetData() const;

	// Return the size
	uint64_t GetSize() const;

protected:

    // Set the data
    void SetData(std::vector<uint8_t>&& _data);

private:

    std::vector<uint8_t> m_Data;
};

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketResource
////////////////////////////////////////////////////////////////////////////////
class PacketResource
{
public:

	// Friend classes
	friend PacketResourceManager;
	friend PacketResourceLoader;
	friend PacketResourceDeleter;
	friend PacketResourceStorage;
	friend PacketResourceInstance;
    friend PacketResourceExternalConstructor;
    friend PacketResourceCreationProxy;
    friend PacketResourceCreationProxyInterface;
    friend PacketResourceKeepAlive;

	template <typename ResourceClass>
	friend class PacketResourceReference;

    enum class ConstructionStatus
    {
        NotConstructed, 
        WaitingExternalConstruction, 
        Constructed,
        ConstructionFailed
    };

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketResource();
	virtual ~PacketResource();

/////////////////////
// VIRTUAL METHODS //
protected: //////////

    // The construct methods, the OnConstruct will be called asynchronously by the resource 
    // manager and the external one can be flagged to be called by the user
    virtual bool OnConstruct(PacketResourceData& _data, uint32_t _buildFlags, uint32_t _flags);
    virtual bool OnExternalConstruct(PacketResourceData& _data, uint32_t _buildFlags, uint32_t _flags, void* _customData);

    // This method is called when this resource is modified (edited), it's guaranteed that it
    // can safely update any internal data without risks of races
    virtual bool OnModification();

	// The OnDelete() method (asynchronous method)
	virtual bool OnDelete(PacketResourceData&) = 0;

    // Return if this resource requires an external construct phase
    virtual bool RequiresExternalConstructPhase() const;

//////////////////
// MAIN METHODS //
public: //////////

	// Return the resource hash
    Hash GetHash() const;

	// Return the data size
	uint32_t GetDataSize() const;

	// Return the total number of references
	uint32_t GetTotalNumberReferences() const;

	// Return the object factory without cast
	PacketResourceFactory* GetFactoryPtr() const;

	// Return the object factory casting to the given template typeclass
	template<typename FactoryClass>
	FactoryClass* GetFactoryPtr() const
	{
		return reinterpret_cast<FactoryClass*>(m_FactoryPtr);
	}

    // Return the current operation mode
    OperationMode GetOperationMode() const;

////////////////////////////
protected: // REPLACEMENT //
////////////////////////////

    // Register another resource as the replacement of this one, this will only take effect on edit mode
    void RegisterReplacingResource(PacketResource* _replacingObject);

    // Return, if set, a resource that must be used instead the current one because it was replaced, this
    // will only take effect on edit mode
    std::optional<PacketResource*> GetReplacingResource() const;

////////////////////
public: // STATUS //
////////////////////

	// Return the status of this resource
	virtual bool IsValid()              const;
	bool IsPendingDeletion()            const;
	bool IsReferenced()                 const;
	bool IsPermanent()                  const;

    /*
    * Return the construction status for this resource
    */
    ConstructionStatus GetConstructionStatus() const;

    /*
    * Returns if the the current resource is waiting on dependencies
    * It's important to noticed that, if a resource is a dependency of another resource and it fails to be
    * constructed, it should not be considered as a dependency anymore, IsValid() for this resource should
    * return false because of this but you should NOT wait on it anymore, query the construction status for
    * resource dependencies via GetConstructionStatus()
    * This function will be called until it returns false, afterwards it will never again be considered, 
    * not causing a major impact on performance
    */
    virtual bool IsWaitingOnDependencies() const;

/////////////////////////
protected: // INTERNAL //
/////////////////////////

	// Begin load, deletion, construct and external construct methods
	bool BeginLoad(bool _isPersistent);
	bool BeginDelete();
    void BeginConstruct();
    void BeginExternalConstruct(void* _data);
    void BeginModifications();

	// Set the hash
	void SetHash(Hash _hash);

	// Set the helper object pointers and the operation mode
    void SetHelperObjects(
        PacketResourceFactory*  _factoryReference,
        PacketResourceManager*  _resourceManager,
        PacketLogger*           _logger,
        OperationMode           _operationMode);

	// Set the build info
	void SetBuildInfo(PacketResourceBuildInfo _buildInfo);

	// Set that this resource is pending deletion
	void SetPendingDeletion();

	// Return the build info
    const PacketResourceBuildInfo& GetBuildInfo() const;

	// Return the data reference, non const because this should be used to externally release the data (factory)
    PacketResourceData& GetDataRef();

    // Increment/decrement the number of references
    void IncrementNumberReferences();
    void DecrementNumberReferences(bool _releaseEnable = true);

///////////////
// VARIABLES //
private: //////

	// Status
	bool m_IsPermanentResource             = false;
	bool m_IsPendingDeletion               = false;
    mutable bool m_IsWaitingOnDependencies = false;
    bool m_WasLoaded                       = false;
    bool m_WasConstructed                  = false;
    bool m_WasExternallyConstructed        = false;
    bool m_ConstructFailed                 = false;

    // This is the index of the current construct phase
    uint32_t m_CurrentConstructPhaseIndex = 0;

	// The total number of direct and indirect references
	std::atomic<uint32_t> m_TotalReferences;

	// The resource data, hash and build info
	PacketResourceData m_Data;
	Hash m_Hash;
	PacketResourceBuildInfo m_BuildInfo;

	// A pointer to the resource factory, the reference manager, the file loader and the logger object
	PacketResourceFactory*  m_FactoryPtr;
    PacketResourceManager*  m_ResourceManagerPtr;
	PacketLogger*           m_LoggerPtr;

    // A resource that is replacing this one
    PacketResource* m_ReplacingResource = nullptr;

	// The current operation mode for the packet system
	OperationMode m_CurrentOperationMode;

    // Prevent thread races using this mutex
    std::mutex m_ResourceMutex;
};

class PacketResourceCreationProxy
{
    friend PacketResourceCreationProxyInterface;

public:

    using OnLoadCallback = std::function<void(PacketResource& _resource, bool _failed)>;

    // Forward a resource linkage, will be called asynchronously by the resource manager
    void ForwardResourceLink(PacketResource* _resource);

    // Return a unique lock that can be used to prevent concurrent access
    std::unique_lock<std::mutex> AcquireLock() const
    {
        return std::move(std::unique_lock<std::mutex>(m_Mutex));
    }

    // This method will update the internal resource variable, it must be called only
    // by the resource manager or by a resource reference, this method doesn't do any
    // synchronization by itself
    void UpdateLinkedResourceVariable(PacketResource** _variablePtr, std::optional<OnLoadCallback> _callback = std::nullopt)
    {
        m_ResourceReferenceVariable = _variablePtr;
        
        if (_callback.has_value())
        {
            m_on_load_callback = std::move(_callback.value());
        }
    }

private:

    // The resource variable that must be updated when the resource is ready
    PacketResource** m_ResourceReferenceVariable = nullptr;

    // The mutex that will prevent concurrent access between the referenced reference and the resource
    // registration
    mutable std::mutex m_Mutex;

    OnLoadCallback m_on_load_callback;
};

class PacketResourceKeepAlive
{
    template <typename ResourceClass>
    friend class PacketResourceReference;

protected:

    // Reference only constructor
    PacketResourceKeepAlive(PacketResource* _resource) :
        m_ResourceObject(_resource)
    {
        m_ResourceObject->IncrementNumberReferences();
    }

public:

    // Default constructor
    PacketResourceKeepAlive() :
        m_ResourceObject(nullptr)
    {
    }

    // Copy constructor
    PacketResourceKeepAlive(const PacketResourceKeepAlive& _other)
    {
        // Reset this reference
        Reset();

        m_ResourceObject = _other.m_ResourceObject;
        if (m_ResourceObject != nullptr)
        {
            m_ResourceObject->IncrementNumberReferences();
        }
    };

    // Assignment operator
    PacketResourceKeepAlive& operator=(const PacketResourceKeepAlive& _other)
    {
        // Reset this reference
        Reset();

        m_ResourceObject = _other.m_ResourceObject;
        if (m_ResourceObject != nullptr)
        {
            m_ResourceObject->IncrementNumberReferences();
        }

        return *this;
    };

    // Move assignment operator
    PacketResourceKeepAlive& operator=(PacketResourceKeepAlive&& _other)
    {
        // Reset this reference
        Reset();

        m_ResourceObject = std::move(_other.m_ResourceObject);
        _other.m_ResourceObject = nullptr;

        return *this;
    }

    // Our move copy operator
    PacketResourceKeepAlive(PacketResourceKeepAlive&& _other)
    {
        // Reset this reference
        Reset();

        m_ResourceObject = std::move(_other.m_ResourceObject);
        _other.m_ResourceObject = nullptr;
    }

    // Reset this pointer, unlinking it
    virtual void Reset()
    {
        // If the resource object is valid
        if (m_ResourceObject != nullptr)
        {
            m_ResourceObject->DecrementNumberReferences();
            m_ResourceObject = nullptr;
        }
    }

    operator bool() const
    {
        return IsValid();
    }

    // Return if this is valid
    bool IsValid() const
    {
        return m_ResourceObject != nullptr;
    }

    // Destructor
    virtual ~PacketResourceKeepAlive()
    {
        // If the resource object is valid
        if (m_ResourceObject != nullptr)
        {
            m_ResourceObject->DecrementNumberReferences();
        }
    }

private:

    PacketResource* m_ResourceObject = nullptr;
};

// The temporary resource reference type
template <typename ResourceClass>
class PacketResourceReference
{
    // Friend classes
    friend PacketResource;
    friend PacketResourceInstance;
    friend PacketResourceManager;

protected:

    // Register a creation proxy, only callable from the resource manager
    void RegisterCreationProxy(PacketResourceCreationProxy* _creationProxy)
    {
        // Set our proxy variable
        m_CreationProxy = _creationProxy;
        UpdateCreationProxy(&m_ResourceObject);
    }

    void RegisterResourceHash(Hash _resource_hash)
    {
        m_resource_hash = _resource_hash;
    }

public:

    // Default constructor
    PacketResourceReference() :
        m_ResourceObject(nullptr)
    {
    }

    // Copy constructor
    PacketResourceReference(const PacketResourceReference& _other)
    {
        // Reset this reference
        Reset();

        m_ResourceObject = _other.m_ResourceObject;
        m_resource_hash  = _other.m_resource_hash;
        if (m_ResourceObject != nullptr)
        {
            m_ResourceObject->IncrementNumberReferences();
        }
    };

    // Assignment operator
    PacketResourceReference& operator=(const PacketResourceReference& _other)
    {
        // Reset this reference
        Reset();

        m_ResourceObject = _other.m_ResourceObject;
        m_resource_hash  = _other.m_resource_hash;
        if (m_ResourceObject != nullptr)
        {
            m_ResourceObject->IncrementNumberReferences();
        }

        return *this;
    };

    // Move assignment operator
    PacketResourceReference& operator=(PacketResourceReference&& _other)
    {
        // Reset this reference
        Reset();

        m_CreationProxy = _other.m_CreationProxy;
        m_resource_hash = _other.m_resource_hash;

        // Make the other proxy target our resource variable
        UpdateCreationProxy(&m_ResourceObject);

        m_ResourceObject = std::move(_other.m_ResourceObject);
        _other.m_ResourceObject = nullptr;
        _other.m_CreationProxy = nullptr;
        _other.m_resource_hash = Hash();

        return *this;
    }

    // Our move copy operator
    PacketResourceReference(PacketResourceReference&& _other)
    {
        // Reset this reference
        Reset();

        m_CreationProxy = _other.m_CreationProxy;
        m_resource_hash = _other.m_resource_hash;

        // Make the other proxy target our resource variable
        UpdateCreationProxy(&m_ResourceObject);

        m_ResourceObject = std::move(_other.m_ResourceObject);
        _other.m_ResourceObject = nullptr;
        _other.m_CreationProxy = nullptr;
        _other.m_resource_hash = Hash();
    }

    // Reset this pointer, unlinking it
    virtual void Reset()
    {
        // Unlink from the proxy, if necessary
        UpdateCreationProxy(nullptr);

        // If the resource object is valid
        if (m_ResourceObject != nullptr)
        {
            m_ResourceObject->DecrementNumberReferences();
            m_ResourceObject = nullptr;
        }

        m_resource_hash = Hash();
    }

    // Operator to use this as a pointer to the resource object
    ResourceClass* operator->() const
    {
        return Get();
    }

    // The get method
    ResourceClass* Get() const
    {
        // If we are on edit mode, check if this resource should be replaced
        if (m_ResourceObject!= nullptr && m_ResourceObject->GetOperationMode() == OperationMode::Plain)
        {
            auto replacingResource = m_ResourceObject->GetReplacingResource();
            if (replacingResource)
            {
                // Increment the ref count for the new resource
                replacingResource.value()->IncrementNumberReferences();

                // Decrement the number of references for the current resource
                m_ResourceObject->DecrementNumberReferences();

                // Set our new resource object
                m_ResourceObject = replacingResource.value();
            }
        }

        return reinterpret_cast<ResourceClass*>(m_ResourceObject);
    }

    /*
    * This function will only update the internal resource hash without afecting
    * the underlying resource, if any
    */
    void SetHashWithoutLoading(Hash _resource_hash)
    {
        RegisterResourceHash(_resource_hash);
    }

    const Hash& GetHash() const
    {
        return m_resource_hash;
    }

    PacketResourceKeepAlive GetKeepAlive() const
    {
        return PacketResourceKeepAlive(m_ResourceObject);
    }

    operator bool() const
    {
        return IsValid();
    }

    // Return if this is valid
    bool IsValid() const
    {
        ResourceClass* resource = Get();
        return resource != nullptr && reinterpret_cast<const PacketResource*>(resource)->IsValid();
    }

    bool HasValidCreationProxy() const
    {
        return m_CreationProxy != nullptr;
    }

    // Destructor
    virtual ~PacketResourceReference()
    {
        // Unlink from the proxy, if necessary
        UpdateCreationProxy(nullptr);

        // If the resource object is valid
        if (m_ResourceObject != nullptr)
        {
            m_ResourceObject->DecrementNumberReferences();
        }
    }

    virtual void OnResourceLoaded(PacketResource& _resource, bool _failed)
    {
    }

private:

    // This method will update our creation proxy, if necessary, to point to
    // a new resource object variable, it can also be used to unlink from it 
    // specifying a nullptr
    // A bool is returned indicating if the creation proxy was updated or not,
    // this can be used to determine if the current resource is still pending
    // to be set by the proxy or if it was already set
    // Optionally pass a second resource variable that will be verified (use on
    // move operations)
    bool UpdateCreationProxy(PacketResource** _resourceVariable, PacketResource** _movingResourceVariable = nullptr)
    {
        if (m_CreationProxy)
        {
            // Acquire a lock from the proxy to prevent it from updating our resource variable at the
            // same time we are releasing this reference
            auto proxyLock = m_CreationProxy->AcquireLock();

            // Check if the proxy already updated our resource
            if ((_resourceVariable == nullptr || *_resourceVariable == nullptr) && (_movingResourceVariable == nullptr || *_movingResourceVariable == nullptr))
            {
                std::optional<PacketResourceCreationProxy::OnLoadCallback> load_callback;
                if (_resourceVariable != nullptr)
                {
                    load_callback = [this](PacketResource& _resource, bool _failed) -> void {OnResourceLoaded(_resource, _failed); };
                }

                // Unlink the proxy by setting the target resource variable to nullptr
                m_CreationProxy->UpdateLinkedResourceVariable(_resourceVariable, std::move(load_callback));
            }
            else
            {
                m_CreationProxy = nullptr;

                return false;
            }

            if (_resourceVariable == nullptr)
            {
                m_CreationProxy = nullptr;
            }

            return true;
        }

        return false;
    }

protected:

    // The resource object
    mutable PacketResource* m_ResourceObject = nullptr;

    // The creation proxy linked
    PacketResourceCreationProxy* m_CreationProxy = nullptr;

    Hash m_resource_hash;
};

// A temporary object that should be used to call the resource OnExternalConstruct method, this
// object will be returned by the resource manager when a thread query about the resources that
// need external construction
class PacketResourceExternalConstructor
{
    friend PacketResourceManager;

protected:

    PacketResourceExternalConstructor(PacketResource* _resource, PacketResourceManager* _manager);

public:

    PacketResourceExternalConstructor(const PacketResourceExternalConstructor&) = delete;
    PacketResourceExternalConstructor& operator=(const PacketResourceExternalConstructor&) = delete;
    PacketResourceExternalConstructor();
    PacketResourceExternalConstructor(PacketResourceExternalConstructor&& _other);
    PacketResourceExternalConstructor& operator=(PacketResourceExternalConstructor&& _other);
    ~PacketResourceExternalConstructor();

    // Externally construct the underlying resource
    void ConstructResource(void* _data);

private:

    // The resource that needs construction and our resource manager
    PacketResource*        m_Resource        = nullptr;
    PacketResourceManager* m_ResourceManager = nullptr;
};

// Packet
PacketDevelopmentNamespaceEnd(Packet)