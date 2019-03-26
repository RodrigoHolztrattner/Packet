////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResource.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "..\PacketConfig.h"

#include <memory>
#include <cstdint>
#include <vector>
#include <atomic>
#include <mutex>

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
class PacketResourceManager;
class PacketResourceLoader;
class PacketResourceDeleter;
class PacketResourceFactory;
class PacketResourceWatcher;
class PacketResourceStorage;
class PacketReferenceManager;
class PacketResourceExternalConstructor;

template <typename ResourceClass>
class PacketResourceReferencePtr;

// The structure that will be used to store the resource data, it works like a vector of uint8_t and is allocation 
// and deallocation must be managed by a factory class
struct PacketResourceData
{
    // Friend classes
    friend PacketResourceLoader;

	// Our constructors
	PacketResourceData();
	PacketResourceData(uint8_t* _data, uint64_t _size);
    PacketResourceData(uint64_t _size);
    PacketResourceData(std::vector<uint8_t> _runtimeData);

	// Copy constructor
	PacketResourceData(PacketResourceData&);

	// The pointer deletion must be done manually
	~PacketResourceData();

	// Our move assignment operator
	PacketResourceData& operator=(PacketResourceData&& _other);

	// Our move copy operator (same as above)
	PacketResourceData(PacketResourceData&& _other);

	// Return the data
	const uint8_t* GetData() const;

	// Return the size
	uint64_t GetSize() const;

	// Allocates memory for this object
	virtual bool AllocateMemory(uint64_t _total);

	// Deallocate this object memory
	virtual void DeallocateMemory();

protected:

    // Return a pointer to this memory enabling it to be written
    uint8_t* GetwritableData() const;

protected:

	uint8_t* m_Data;
	uint64_t m_Size;

private:

    std::vector<uint8_t> m_RuntimeData;
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
	friend PacketResourceWatcher;
	friend PacketResourceStorage;
	friend PacketResourceInstance;
    friend PacketResourceExternalConstructor;

	template <typename ResourceClass>
	friend class PacketResourceReferencePtr;

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketResource();
	virtual ~PacketResource();

/////////////////////
// VIRTUAL METHODS //
protected: //////////

	// The OnLoad() method (asynchronous method)
	virtual bool OnLoad(PacketResourceData& _data, uint32_t _buildFlags, uint32_t _flags) = 0;

	// The OnDelete() method (asynchronous method)
	virtual bool OnDelete(PacketResourceData&) = 0;

    // Return if this resource requires an external construct phase
    virtual bool RequiresExternalConstructPhase() const;

    // The construct methods, the OnConstruct will be called asynchronously by the resource 
    // manager and the external one can be flagged to be called by the user
    virtual void OnConstruct();
    virtual void OnExternalConstruct(void* _data);

    // This method is called when this resource is modified (edited), it's guaranteed that it
    // can safely update any internal data without risks of races
    virtual void OnModification();

//////////////////
// MAIN METHODS //
public: //////////

	// Return the resource hash
    Hash GetHash() const;

	// Return the data size
	uint32_t GetDataSize() const;

	// Return the total number of instances that directly or indirectly references this resource
	uint32_t GetTotalNumberReferences()         const;
	uint32_t GetTotalNumberDirectReferences()   const;
	uint32_t GetTotalNumberIndirectReferences() const;

	// Return the object factory without cast
	PacketResourceFactory* GetFactoryPtr() const;

	// Return the object factory casting to the given template typeclass
	template<typename FactoryClass>
	FactoryClass* GetFactoryPtr() const
	{
		return reinterpret_cast<FactoryClass*>(m_FactoryPtr);
	}

//////////////////////////////////
public: // PHYSICAL DATA UPDATE //
//////////////////////////////////

	// This method will set this resource to ignore physical changes on its data, if the resource file happens to be 
	// modified or any call to UpdateResourcePhysicalData() method won't be resulting in the destruction of this 
	// resource and in a creation of a new one, this method will only works when on edit mode for the packet system 
	void IgnoreResourcePhysicalDataChanges();

	// Update this resource physical data, overwriting it. This method will only works if the packet system is operating on 
	// edit mode, by default calling this method will result in a future deletion of this resource object and in a creation
	// of a new resource object with the updated data, if the user needs to update the data at runtime multiple times is 
	// recommended to batch multiple "data updates" and call this method once in a while (only call this method when there is 
	// a real need to actually save the data)
	bool UpdateResourcePhysicalData(const uint8_t* _data, uint64_t _dataSize);
	bool UpdateResourcePhysicalData(PacketResourceData& _data);

////////////////////////////////////////
public: // PHYSICAL RESOURCE REFERECE //
////////////////////////////////////////

	// Register a physical reference from this resource to another one path, doing this will allows the reference system to 
	// detect filename changes and modifications, allowing it to update this resource to point to the new resources, this 
	// method will return false if the current operation mode is different from the edit one or if this isn't a debug build.
	// The user must provide the location where the target resource hash data is located INSIDE this resource data, the 
	// location must indicates where this hash can be found when reading the resource file (doing a seek + this position).
	bool RegisterPhysicalResourceReference(Hash _targetResourceHash, uint64_t _hashDataLocation);

	// This method will clear all physical resource references for this current resource, the same rules above also apply to
	// this method (return false if the operation mode isn't the edit one or this isn't a debug build)
	bool ClearAllPhysicalResourceReferences();

	// This method will verify all resource physical references this resource have, checking if they exist, it's possible to 
	// pass a ReferenceFixer as argument that determine a behaviour to find any missing reference and fix it. For more info 
	// take a look at the documentation, use this method with caution.
	// The same rules above also apply to this method (return false if the operation mode isn't the edit one or this isn't 
	// a debug build)
	bool VerifyPhysicalResourceReferences(ReferenceFixer _fixer = ReferenceFixer::None, bool _allOrNothing = true);

////////////////////
public: // STATUS //
////////////////////

	// Return the status of this resource
	bool IsReady()                const;
	bool IsPendingDeletion()      const;
	bool IsReferenced()           const;
	bool IsDirectlyReferenced()   const;
	bool IsIndirectlyReferenced() const;
	bool IsPermanent()            const;
    bool IsRuntime()              const;
    bool IsPendingModifications() const;
    bool IgnorePhysicalDataChanges() const;

/////////////////////////////////////
protected: // INSTANCE REFERENCING //
/////////////////////////////////////

	// Make a instance reference/remove this resource as its internal resource
	void MakeInstanceReference(PacketResourceInstance* _instance);
	void RemoveInstanceReference(PacketResourceInstance* _instance);

    // Make all instances that depends on this resource to point to another resource, decrementing the total 
    // number of references to zero, this method must be called when inside the update phase on the resource 
    // manager so no race conditions will happen. This method will only do something when on debug builds
    void RedirectInstancesToResource(PacketResource* _newResource);

    // This method will check if all instances that depends on this resource are totally constructed and ready
    // to be used, this method only works on debug builds and it's not intended to be used on release builds, 
    // also this method must be called when inside the update method on the PacketResourceManager class
    bool AreInstancesReadyToBeUsed() const;

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
	void SetHelperObjects(PacketResourceFactory* _factoryReference,
                          PacketReferenceManager* _referenceManager, 
                          PacketResourceManager* _resourceManager,
                          PacketFileLoader* _fileLoader, 
                          PacketLogger* _logger, 
                          OperationMode _operationMode);

	// Set the build info
	void SetBuildInfo(PacketResourceBuildInfo _buildInfo, 
                      bool _isRuntimeResource);

	// Set that this resource is pending deletion
	void SetPendingDeletion();

    // Set that this resource has pending modification, this will lock the modification mutex and prevent
    // any new resource from being created, after this call when the resource has 0 indirect references
    // it should be able to handle modifications safely
    // This method will also add this resource into the "pending modification queue" on the resource
    // manager, this is ok since the manager won't perform the modifications until this resource get 0
    // indirect references, it will have at least one because this method should be called from a reference
    // object.
    void SetPendingModifications();

	// Return the build info
    const PacketResourceBuildInfo& GetBuildInfo() const;

	// Return the data reference, non const because this should be used to externally release the data (factory)
    PacketResourceData& GetDataRef();

    // Increment/decrement the number of direct references
    void IncrementNumberDirectReferences();
    void DecrementNumberDirectReferences();

    // Increment/decrement the number of indirect references
    void IncrementNumberIndirectReferences();
    void DecrementNumberIndirectReferences();

///////////////
// VARIABLES //
private: //////

	// Status
	bool m_IsPermanentResource       = false;
	bool m_IgnorePhysicalDataChanges = false;
	bool m_IsPendingDeletion         = false;
    bool m_WasLoaded                 = false;
    bool m_WasConstructed            = false;
    bool m_WasExternallyConstructed  = false;
    bool m_IsRuntimeResource         = false;
    bool m_IsPendingModifications    = false;

    // This is the index of the current construct phase
    uint32_t m_CurrentConstructPhaseIndex = 0;

	// The total number of direct and indirect references
	std::atomic<uint32_t> m_TotalDirectReferences;
	std::atomic<uint32_t> m_TotalIndirectReferences;

	// The resource data, hash and build info
	PacketResourceData m_Data;
	Hash m_Hash;
	PacketResourceBuildInfo m_BuildInfo;

	// A pointer to the resource factory, the reference manager, the file loader and the logger object
	PacketResourceFactory*  m_FactoryPtr;
    PacketReferenceManager* m_ReferenceManagerPtr;
    PacketResourceManager*  m_ResourceManagerPtr;
    PacketFileLoader*       m_FileLoaderPtr;
	PacketLogger*           m_LoggerPtr;

	// The current operation mode for the packet system
	OperationMode m_CurrentOperationMode;

	// This is a vector with all instances that uses this object + a mutex to prevent multiple writes 
	// at the same time, those variables exist only on debug builds and they offer functionality to 
	// the runtime detection of resource file changes, providing a way to update all instances that 
	// points to a given resource object
	std::vector<PacketResourceInstance*> m_InstancesThatUsesThisResource;

    // This is the modifications mutex, it is used to prevent thread races when the modification
    // flag is active, preventing any new reference from being created
    std::mutex m_ResourceModificationMutex;
};

// The temporary resource reference type
template <typename ResourceClass>
class PacketResourceReferencePtr
{
    // Friend classes
    friend PacketResource;
    friend PacketResourceInstance;

protected:

    // Constructor used by the instance class
    PacketResourceReferencePtr(ResourceClass* _resource) :
        m_ResourceObject(_resource),
        m_PacketResourceObject(_resource)
    {
        // Increment the total number of temporary references for the resource
        m_PacketResourceObject->IncrementNumberIndirectReferences();
    }

public:

    // Default constructor
    PacketResourceReferencePtr() :
        m_ResourceObject(nullptr),
        m_PacketResourceObject(nullptr)
    {
    }

    // Copy constructor disabled
    PacketResourceReferencePtr(const PacketResourceReferencePtr&) = delete;

    // Assignment operator
    PacketResourceReferencePtr& operator=(const PacketResourceReferencePtr&) = delete;

    // Move assignment operator
    PacketResourceReferencePtr& operator=(PacketResourceReferencePtr&& _other)
    {
        m_ResourceObject = std::move(_other.m_ResourceObject);
        m_PacketResourceObject = std::move(_other.m_PacketResourceObject);
        
        return *this;
    }

    // Our move copy operator
    PacketResourceReferencePtr(PacketResourceReferencePtr&& _other)
    {
        m_ResourceObject = std::move(_other.m_ResourceObject);
        m_PacketResourceObject = std::move(_other.m_PacketResourceObject);
    }

    // Reset this pointer, unlinking it
    virtual void Reset()
    {
        // If the resource object is valid
        if (m_ResourceObject != nullptr)
        {
            m_PacketResourceObject->DecrementNumberIndirectReferences();
            m_PacketResourceObject = nullptr;
            m_ResourceObject = nullptr;
        }
    }

    // Operator to use this as a pointer to the resource object
    ResourceClass* operator->() const
    {
        return m_ResourceObject;
    }

    // The get method
    ResourceClass* Get()
    {
        return m_ResourceObject;
    }

    // Return if this is valid
    bool IsValid()
    {
        return m_ResourceObject != nullptr;
    }

    // Destructor
    virtual ~PacketResourceReferencePtr()
    {
        // If the resource object is valid
        if (m_ResourceObject != nullptr)
        {
            m_PacketResourceObject->DecrementNumberIndirectReferences();
            m_PacketResourceObject = nullptr;
            m_ResourceObject = nullptr;
        }
    }

protected:

    // The resource object
    ResourceClass*  m_ResourceObject = nullptr;
    PacketResource* m_PacketResourceObject = nullptr;
};

// The temporary editable resource reference type
template <typename ResourceClass>
class PacketEditableResourceReferencePtr : public PacketResourceReferencePtr<ResourceClass>
{
protected:

    // Constructor used by the instance class
    PacketEditableResourceReferencePtr(PacketResourceInstance* _instance, 
                                       ResourceClass* _resource)
        : PacketResourceReferencePtr<ResourceClass>(_resource)
    {
        _resource->SetPendingModifications();
    }

public:

    // Default constructor
    PacketEditableResourceReferencePtr() 
        : PacketResourceReferencePtr<ResourceClass>() {}
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

    PacketResourceExternalConstructor() = delete;
    PacketResourceExternalConstructor(const PacketResourceExternalConstructor&) = delete;
    PacketResourceExternalConstructor& operator=(const PacketResourceExternalConstructor&) = delete;
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