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
		// Check if the resoruce instance is valid (only invalid if someone took the ownership using a 
		// move semantic, if we never initialized a resource instance with it or if we manually reseted
		// this pointer)
		if (m_ResourceInstance != nullptr)
		{
			// The resource instance is always valid and this object deletion is happening without the
			// packet system update active so we are free to directly do changes on the instance object.
			// The instance unlink will take ownership over our unique_ptr
			m_ResourceInstance->InstanceUnlink(m_ResourceInstance);
		}
	}

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
			m_ResourceInstance->InstanceUnlink(m_ResourceInstance);
			m_ResourceInstance = nullptr;
		}
	}

protected:

	// This method is called by a resource manager object (our only friended class) and will set our 
	// instance pointer to reference a valid instance object, creating a link
	void InstanceLink(std::unique_ptr<PacketResourceInstance>& _instancePtr)
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
	PacketResourceInstance(Hash& _hash, PacketResourceManager* _resourceManager, PacketResourceFactory* _factoryPtr);

	// Disable the copy constructor so we won't be making so many mistakes :)
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
	bool IsReady();

	// Create a temporary reference
	template <typename ResourceClass>
	PacketResourceReferencePtr<ResourceClass> GetResourceReference()
	{
		// Increment the total number of temporary references
		m_ReferenceObject->MakeTemporaryReference();

		return PacketResourceReferencePtr<ResourceClass>(static_cast<ResourceClass*>(m_ReferenceObject));
	}

///////////////////////
protected: // STATUS //
///////////////////////

	// Return if all dependencies are fulfilled
	bool AreDependenciesFulfilled();

	// Return if this instance is locked
	bool IsLocked();

//////////////////////////////////
protected: // SELF INTERNAL USE //
//////////////////////////////////

	// Add a dependency to another instance
	void AddInstanceDependency(PacketResourceInstance& _instance);

	// Return if in case this instance has another one that depends on it, if this one is locked. In case there is no
	// dependency, return false
	bool InstanceDependencyIsLocked();

private:

	// Fulfill a dependency
	void FulfillDependency(PacketResourceInstance* _instance);

/////////////////////////////
protected: // EXTERNAL USE //
/////////////////////////////

	// This method must be called from a instance ptr and will cause us to unlink from it, setting this instance to unnused 
	// and in the future making this instance to be released
	void InstanceUnlink(std::unique_ptr<PacketResourceInstance>& _instanceUniquePtr);

	// Return the resource
	PacketResource* GetResource();

	// Return the resource
	template<typename ResourceClass>
	PacketResource* GetResource()
	{
		assert(m_ReferenceObject != nullptr);
		return reinterpret_cast<ResourceClass*>(m_ReferenceObject);
	}

	// Return the object factory casting to the given template typeclass
	template<typename FactoryClass>
	FactoryClass* GetFactoryPtr()
	{
		return m_ReferenceObject->GetFactoryPtr<FactoryClass>();
	}

protected:

	// Unlock this instance to be used by the world
	void Unlock();

	// Begin the construction of this instance
	void BeginConstruction();

	// Reset this instance, clearing its status, this method must be called by a resource object 
	// that is being replaced by a newer version, its ensured that this will only be called when 
	// all dependencies for this instance are fulfilled, also if there is another instance that 
	// depends on this one, it's ensured that this instance was also constructed and ready, 
	// recursivelly
	void ResetInstance();

	// Set the peasant object reference
	void SetObjectReference(PacketResource* _objectReference);

/////////////////////
// VIRTUAL METHODS //
protected: //////////

	// Called when the resource object was loaded, here its possible to set internal variables 
	// and all other instance dependencies for this one
	virtual void OnConstruct() = 0;

	// Called when all instance dependencies are fulfilled
	virtual void OnDependenciesFulfilled() = 0;

	// Called when this instance object was reseted and its status must be cleaned, this means that 
	// this instance will be constructed again, also if there is another instance that depends on 
	// this one it will be put on locked state until this instance is reconstructed.
	// It's ensured that this will only be called when all dependencies for this instance are fulfilled, 
	// also if there is another instance that depends on this one, it's ensured that this instance was 
	// also constructed and ready, recursivelly
	virtual void OnReset() = 0;

///////////////
// VARIABLES //
private: //////

	// If this instance is linked with its owned ptr
	bool m_IsLinked;

	// If this instance is locked
	bool m_IsLocked;

	// The object we are referencing
	PacketResource* m_ReferenceObject;

	// The dependency count
	std::atomic<uint32_t> m_DependencyCount;

	// The instance that depends on this one
	PacketResourceInstance* m_LinkedInstanceDependency;

	// The hash object
	Hash m_Hash;

	// The resource manager reference
	PacketResourceManager* m_ResourceManagerPtr;

	// The factory ptr
	PacketResourceFactory* m_FactoryPtr;
};

// Packet
PacketDevelopmentNamespaceEnd(Packet)
