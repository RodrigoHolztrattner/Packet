////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceInstance.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "..\PacketConfig.h"
#include "PacketResource.h"

#include <cstdint>
#include <vector>
#include <atomic>
#include <cassert>
#include <set>

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
class PacketResourceManager;
class PacketResourceFactory;
class PacketResourceInstance;

/////////////
// STRUCTS //
/////////////

// The resource instance pointer
template <typename InstanceClass>
struct PacketResourceInstancePtr
{
	// Friend classes
	friend PacketResourceManager;

	// Default constructor
	PacketResourceInstancePtr() = default;

	// The destructor must unlink with the resource instance
	~PacketResourceInstancePtr()
	{
		// Check if the resource instance is valid (only invalid if someone took the ownership using a 
		// move semantic, if we never initialized a resource instance with it or if we manually reseted
		// this pointer)
		if (m_ResourceInstance != nullptr)
		{
			// The resource instance is always valid and this object deletion is happening without the
			// packet system update active so we are free to directly do changes on the instance object.
			// The instance unlink will take ownership over our unique_ptr
            auto instancePtr = m_ResourceInstance.get();
            instancePtr->InstanceUnlink(std::move(m_ResourceInstance));
		}
	}

    PacketResourceInstancePtr& operator=(PacketResourceInstancePtr&& _other) = delete;
    /*
	// Our move assignment operator that will take the ownership from the given instance pointer
	PacketResourceInstancePtr& operator=(PacketResourceInstancePtr&& _other) 
	{
		// We can't assign to the same object
		if (this != &_other)
		{
			// Acquire the other resource instance ptr
			m_ResourceInstance = std::move(_other.m_ResourceInstance);
		}

		return *this;
	}
    */

	// Our move copy operator (same as above)
	PacketResourceInstancePtr(PacketResourceInstancePtr&& _other)
	{
		// Acquire the other resource instance ptr
        m_ResourceInstance = std::move(_other.m_ResourceInstance);
	}

	// Operator to use this as a pointer to the instance object
	InstanceClass* operator->() const
	{
		return static_cast<InstanceClass*>(m_ResourceInstance.get());
	}

	// Instance class operator
	operator InstanceClass&()
	{
		return *static_cast<InstanceClass*>(m_ResourceInstance.get());
	}

    // Return the underlying instance object
    PacketResourceInstance* Get() const
    {
        return m_ResourceInstance.get();
    }

	// Bool operator so we can use it on boolean expressions directly
	operator bool() const
	{
		return m_ResourceInstance != nullptr;
	}

	// Reset this pointer, unlinking it
	void Reset()
	{
		// If our resource instance is valid
		if (m_ResourceInstance != nullptr)
		{
			// Unlink and reset it
            PacketResourceInstance* instancePtr = m_ResourceInstance.get();
            instancePtr->InstanceUnlink(std::move(m_ResourceInstance));
		}
	}

protected:

	// This method is called by a resource manager object (our only friended class) and will set our 
	// instance pointer to reference a valid instance object, creating a link
	void InstanceLink(std::unique_ptr<PacketResourceInstance> _instancePtr)
	{
		m_ResourceInstance = std::move(_instancePtr);
	}

private:

	// The resource instance ptr (always valid until this object is deleted, move operations are allowed)
	std::unique_ptr<PacketResourceInstance> m_ResourceInstance = nullptr;
};

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketResourceInstance
////////////////////////////////////////////////////////////////////////////////
class PacketResourceInstance
{
public:

	// Friend classes
	friend PacketResourceManager;
	friend PacketResource;
	friend PacketResourceFactory;

	template <typename InstanceClass>
	friend struct PacketResourceInstancePtr;

//////////////////
// CONSTRUCTORS //
protected: ///////

	// The only one allowed to create a resource instance is the PacketResourceFactory, here we will 
	// set the hash and a pointer to the resource manager object
	PacketResourceInstance(Hash& _hash,
                           PacketResourceManager* _resourceManager,
                           PacketResourceFactory* _factoryPtr);

	// Disable the copy constructor so we won't be making mistakes :)
	PacketResourceInstance(const PacketResourceInstance& _other) = delete;

public:

	// Destructor
	virtual ~PacketResourceInstance();

//////////////////
// MAIN METHODS //
public: //////////

	// This method will return true if first, this instance is linked with its owning ptr (I really hope you are calling this 
	// using the instance ptr object or we will probably have some problems!), if the target resource was created, loaded and 
	// synchronized, and third if this instance was constructed with all of its dependencies fulfilled
	bool IsReady() const;

	// Create a temporary reference from the internal resource, this will prevent it from being deleted even if this
    // instance is released
	template <typename ResourceClass>
	PacketResourceReferencePtr<ResourceClass> GetResourceReference() const
	{
        std::lock_guard<std::mutex> lock(m_SafetyMutex);

		return PacketResourceReferencePtr<ResourceClass>(static_cast<ResourceClass*>(m_ReferenceObject));
	}

    // Create a temporary reference from the internal resource, allowing it to me modified
    // After the editable reference goes out of scope or is explicit reseted, the underlying resource
    // will be enqueued to be updated
    // During the lifetime of this object, the resource will be locked and the only way to use it
    // will be by the reference
    template <typename ResourceClass>
    PacketEditableResourceReferencePtr<ResourceClass> GetEditableResourceReference() const
    {
        std::lock_guard<std::mutex> lock(m_SafetyMutex);
 
        return PacketEditableResourceReferencePtr<ResourceClass>(this, static_cast<ResourceClass*>(m_ReferenceObject));
    }

///////////////////////
protected: // STATUS //
///////////////////////

	// Return if all dependencies are fulfilled
	bool AreDependenciesFulfilled() const;

//////////////////////////////////
protected: // SELF INTERNAL USE //
//////////////////////////////////

	// Add a dependency to another instance
	void AddInstanceDependency(PacketResourceInstance& _instance);

	// Return the ready status for the instance that we are dependent (if we have one, if we don't depend
    // on any instance return true)
	bool InstanceDependencyIsReady() const;

    // Return the instance factory pointer
    PacketResourceFactory* GetFactoryPtr() const;

private:

	// Fulfill a dependency
	void FulfillDependency(PacketResourceInstance* _instance);

/////////////////////////////
protected: // EXTERNAL USE //
/////////////////////////////

	// This method must be called from a instance ptr and will cause us to unlink from it, setting this instance to unused 
	// and in the future making this instance be released
	void InstanceUnlink(std::unique_ptr<PacketResourceInstance> _instanceUniquePtr);

	// Return the resource
	PacketResource* GetResource() const;

	// Return the resource
	template<typename ResourceClass>
	ResourceClass* GetResource() const
	{
		return reinterpret_cast<ResourceClass*>(m_ReferenceObject);
	}

	// Return the object factory casting to the given template typeclass
	template<typename FactoryClass>
	FactoryClass* GetFactoryPtr() const
	{
		return m_ReferenceObject->GetFactoryPtr<FactoryClass>();
	}

protected:

	// Begin the construction/deletion of this instance
	void BeginConstruct();
    void BeginDelete();

	// This method will gather the top parent recursively
	PacketResourceInstance* GatherTopParentInstanceRecursively();

	// Set the peasant object reference
	void SetObjectReference(PacketResource* _objectReference);

    // Lock and unlock this instance usage, preventing concurrent access
    void LockUsage();
    void UnlockUsage();

/////////////////////
// VIRTUAL METHODS //
protected: //////////

	// Called when the resource object was loaded, here its possible to set internal variables 
	// and all other instance dependencies for this one
	virtual void OnConstruct() = 0;

	// Called when all instance dependencies are fulfilled
	virtual void OnDependenciesFulfilled() = 0;

    // Called when this instance must release its internal data, it could be in deletion process
    // or being reseted due a resource replacement
    virtual void OnDelete() = 0;

///////////////
// VARIABLES //
private: //////

    // The safety mutex used to lock the access to this instance if modifications are being made
    mutable std::mutex m_SafetyMutex;

    // The instance data
    bool                    m_WasLinked                 = false;
    bool                    m_WasConstructed           = false;
	PacketResource*         m_ReferenceObject          = nullptr;
	std::atomic<uint32_t>   m_DependencyCount          = 0;
	PacketResourceInstance* m_LinkedInstanceDependency = nullptr;
	Hash                    m_Hash;

	// The resource manager and factory ptrs
	PacketResourceManager* m_ResourceManagerPtr = nullptr;
	PacketResourceFactory* m_FactoryPtr         = nullptr;
};

// Packet
PacketDevelopmentNamespaceEnd(Packet)
