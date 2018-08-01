////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceInstance.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResourceInstance.h"
#include "PacketResourceFactory.h"
#include "PacketResourceManager.h"

// Using namespace Peasant
PacketUsingDevelopmentNamespace(Packet)

PacketResourceInstance::PacketResourceInstance(Hash _hash, PacketResourceManager* _resourceManager, PacketResourceFactory* _factoryPtr)
{
	// Set linked to true
	m_IsLinked = true;

	// Set the hash, the resource manager ptr and the factory ptr
	m_Hash = _hash;
	m_ResourceManagerPtr = _resourceManager;
	m_FactoryPtr = _factoryPtr;

	// Set the initial data
	m_IsLocked = true;
	m_DependencyCount = 0;
	m_LinkedInstanceDependency = nullptr;
	m_ReferenceObject = nullptr;
}

PacketResourceInstance::~PacketResourceInstance()
{
}

void PacketResourceInstance::AddInstanceDependency(PacketResourceInstance& _instance)
{
	// Increment the dependency count
	m_DependencyCount++;

	// Set the instance dependency
	_instance.m_LinkedInstanceDependency = this;
}

bool PacketResourceInstance::InstanceDependencyIsLocked()
{
	// No dependency
	if (m_LinkedInstanceDependency == nullptr)
	{
		return false;
	}

	return m_LinkedInstanceDependency->IsLocked();
}

PacketResource* PacketResourceInstance::GetObjectPtr()
{
	assert(m_ReferenceObject != nullptr);
	return m_ReferenceObject;
}

void PacketResourceInstance::SetObjectReference(PacketResource* _objectReference)
{
	m_ReferenceObject = _objectReference;
}

bool PacketResourceInstance::AreDependenciesFulfilled()
{
	return m_DependencyCount == 0;
}

bool PacketResourceInstance::IsLocked()
{
	return m_IsLocked;
}

/*
bool PacketResourceInstance::RequestDuplicate(PacketResourceInstance& _other)
{
	// Check if this instance was completly loaded
	assert(m_IsLocked || !AreDependenciesFulfilled() || !WasLoaded());

	// Request a new object for this instance
	return m_ObjectManager->RequestObject(this, m_Hash, m_FactoryPtr, false);
}
*/

void PacketResourceInstance::Unlock()
{
	assert(m_IsLocked == true);
	assert(m_DependencyCount == 0);
	assert(m_ReferenceObject != nullptr);
	assert(m_ReferenceObject->IsReady());

	m_IsLocked = false;
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

		// Unlock this instance
		Unlock();
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
	// DETAIL: There is no need to check if the dependency count is zero because it 
	// doesn't matters

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
		// Set the initial data
		m_IsLocked = true;

		// Make this instance be reconstructed in the future
		m_ResourceManagerPtr->ReconstructInstance(this);
	}

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

		// Unlock this instance
		Unlock();
	}
}