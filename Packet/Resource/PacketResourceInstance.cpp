////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceInstance.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResourceInstance.h"
#include "PacketResourceFactory.h"
#include "PacketResourceManager.h"

// Using namespace Peasant
PacketUsingDevelopmentNamespace(Packet)

PacketResourceInstance::PacketResourceInstance(Hash& _hash, 
                                               PacketResourceManager* _resourceManager, 
                                               PacketResourceFactory* _factoryPtr)
{
	// Set linked to true
	m_WasLinked = true;

	// Set the hash, the resource manager ptr and the factory ptr
	m_Hash = _hash;
	m_ResourceManagerPtr = _resourceManager;
	m_FactoryPtr = _factoryPtr;

	// Set the initial data
	m_DependencyCount = 0;
	m_LinkedInstanceDependency = nullptr;
	m_ReferenceObject = nullptr;
}

PacketResourceInstance::~PacketResourceInstance()
{
}

bool PacketResourceInstance::IsReady() const
{
    std::lock_guard<std::mutex> lock(m_SafetyMutex);

	// We only need to check if we are locked and if the reference object is pending replacement, if we are unlocked the reference object
	// is valid (so no need to check the resource ptr) and it is ready to be used, the only case it won't be ok for us is if it's pending
	// replacement, in this case we shouldn't use it until the resource is totally replaced
	return m_WasConstructed && m_ReferenceObject != nullptr && m_ReferenceObject->IsReady();
}

void PacketResourceInstance::AddInstanceDependency(PacketResourceInstance& _instance)
{
	// Increment the dependency count
	m_DependencyCount++;

	// Set the instance dependency
	_instance.m_LinkedInstanceDependency = this;
}

bool PacketResourceInstance::InstanceDependencyIsReady() const
{
	// No dependency
	if (m_LinkedInstanceDependency == nullptr)
	{
		return true;
	}

	return m_LinkedInstanceDependency->IsReady();
}

PacketResourceFactory* PacketResourceInstance::GetFactoryPtr() const
{
    return m_FactoryPtr;
}

void PacketResourceInstance::InstanceUnlink(std::unique_ptr<PacketResourceInstance> _instanceUniquePtr)
{
    std::lock_guard<std::mutex> lock(m_SafetyMutex);

	// Set linked to false
	m_WasLinked = false;
	
	// Use the resource manager to release this instance
	m_ResourceManagerPtr->ReleaseInstanceOnUnlink(std::move(_instanceUniquePtr));
}

PacketResource* PacketResourceInstance::GetResource() const
{
    std::lock_guard<std::mutex> lock(m_SafetyMutex);

	return m_ReferenceObject;
}

void PacketResourceInstance::SetObjectReference(PacketResource* _objectReference)
{
	m_ReferenceObject = _objectReference;
}

void PacketResourceInstance::LockUsage()
{
    m_SafetyMutex.lock();
}

void PacketResourceInstance::UnlockUsage()
{
    m_SafetyMutex.unlock();
}

bool PacketResourceInstance::AreDependenciesFulfilled() const
{
	return m_DependencyCount == 0;
}

void PacketResourceInstance::BeginConstruction()
{
	assert(m_DependencyCount == 0);

	// Call the OnConstruct() method
	OnConstruct();

	// Check if this instance has any dependency
	if (m_DependencyCount == 0)
	{
		// Call the OnDependenciesFulfilled() method
		OnDependenciesFulfilled();

        // Set that this instance was constructed
        m_WasConstructed = true;
	}

	// If some instance depends on this one
	if (m_LinkedInstanceDependency != nullptr)
	{
		// Fulfill the dependency
		m_LinkedInstanceDependency->FulfillDependency(this);
	}
}

void PacketResourceInstance::ResetInstance()
{
    // We should not be able to acquire a lock to the internal mutex because this
    // method should only be called when this instance was previously locked
    assert(!m_SafetyMutex.try_lock());

	// If there is a dependency that depends on this one, reset. 
	if (m_LinkedInstanceDependency != nullptr)
	{
		// Reset it, this will probably release this instance because if there is a 
		// dependency, it must have a pointer to this instance, so calling the 
		// OnReset() method we expect it to release this instance
		m_LinkedInstanceDependency->ResetInstance();
	}
	else
	{
		// Make this instance be reconstructed in the future
		m_ResourceManagerPtr->ReconstructInstance(this);
	}

    // Reset internal data
    m_WasConstructed = false;
    m_LinkedInstanceDependency = nullptr;

	// Call the on reset virtual method
	OnReset();
}

void PacketResourceInstance::FulfillDependency(PacketResourceInstance* _instance)
{
	assert(m_DependencyCount != 0);

	// Subtract one from the dependency count
	m_DependencyCount--;

	// Check if we fulfilled all dependencies
	if (m_DependencyCount == 0)
	{
		// Call the OnDependenciesFulfilled() method
		OnDependenciesFulfilled();

        // Set that this instance was constructed
        m_WasConstructed = true;
	}
}