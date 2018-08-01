////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResource.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResource.h"
#include "PacketResourceInstance.h"
#include "PacketResourceFactory.h"

#include <cassert>

// Using namespace Peasant
PacketUsingDevelopmentNamespace(Packet)

//
//
//

PacketResourceData::PacketResourceData() : m_Data(nullptr), m_Size(0)
{
}

PacketResourceData::PacketResourceData(uint8_t* _data, uint64_t _size) : m_Data(_data), m_Size(_size) 
{
}

PacketResourceData::PacketResourceData(uint64_t _size)
{
	// Allocate the initial memory
	bool result = AllocateMemory(_size);
	assert(result);
}

PacketResourceData::PacketResourceData(PacketResourceData& _other)
{
	m_Data = _other.m_Data;
	m_Size = _other.m_Size;
	_other.m_Data = nullptr;
	_other.m_Size = 0;
}

PacketResourceData::~PacketResourceData()
{
	assert(m_Data == nullptr);
	assert(m_Size == 0);
}

PacketResourceData& PacketResourceData::operator=(PacketResourceData&& _other)
{
	// We can't assign to the same object
	if (this != &_other)
	{
		// Acquire the other resource instance ptr
		m_Data = _other.m_Data;
		m_Size = _other.m_Size;
		m_Data = nullptr;
		m_Size = 0;
	}

	return *this;
}

PacketResourceData::PacketResourceData(PacketResourceData&& _other)
{
	// Acquire the other resource instance ptr
	m_Data = _other.m_Data;
	m_Size = _other.m_Size;
	m_Data = nullptr;
	m_Size = 0;
}

uint8_t* PacketResourceData::GetData()
{
	return m_Data;
}

uint64_t PacketResourceData::GetSize()
{
	return m_Size;
}

bool PacketResourceData::AllocateMemory(uint64_t _total)
{
	m_Data = new uint8_t[unsigned int(_total)];
	if (m_Data == nullptr)
	{
		return false;
	}

	m_Size = _total;

	return true;
}

void PacketResourceData::DeallocateMemory()
{
	delete[] m_Data;
	m_Data = nullptr;
	m_Size = 0;
}

//
//
//

PacketResource::PacketResource()
{
	// Set the initial data
	m_UpdateConditionFlags = 0;
	m_DataValid = false;
	m_WasSynchronized = false;
	m_IsPersistent = false;
	m_IsPendingReplacement = false;
	m_TotalDirectReferences = 0;
	m_TotalIndirectReferences = 0;
}

PacketResource::~PacketResource()
{
	assert(m_TotalDirectReferences == 0);
	assert(m_TotalIndirectReferences == 0);
}

bool PacketResource::BeginLoad(bool _isPersistent)
{
	// Check if the data is valid
	if (m_Data.GetSize() == 0)
	{
		return false;
	}

	// Set if this object is persistent
	m_IsPersistent = _isPersistent;

	// Call the Onload() method
	if (!OnLoad(m_Data))
	{
		return false;
	}

	// Set data valid and loaded
	m_DataValid = true;

	return true;
}

bool PacketResource::BeginDelete()
{
	// Check if we have some data to delete
	if (m_Data.GetSize() == 0)
	{
		return false;
	}

	// Unsynchronize this object
	m_WasSynchronized = false;

	// Set data invalid
	m_DataValid = false;

	// Call the OnDelete() method
	if (!OnDelete(m_Data))
	{
		return false;
	}

	// The data must still be valid
	if (m_Data.GetSize() == 0)
	{
		return false;
	}

	return true;
}

bool PacketResource::BeginSynchronization()
{
	assert(!m_WasSynchronized);

	// Call the OnSynchronization() method
	OnSynchronization();

	// Set synchronized
	m_WasSynchronized = true;

	return true;
}

bool PacketResource::IsReady()
{
	return m_DataValid && m_WasSynchronized;
}

bool PacketResource::IsPendingReplacement()
{
	return m_IsPendingReplacement;
}

bool PacketResource::IsReferenced()
{
	return m_TotalDirectReferences > 0 || m_TotalIndirectReferences > 0;
}

bool PacketResource::IsDirectlyReferenced()
{
	return m_TotalDirectReferences > 0;
}

bool PacketResource::IsIndirectlyReferenced()
{
	return m_TotalIndirectReferences > 0;
}

bool PacketResource::IsPersistent()
{
	return m_IsPersistent;
}

Hash PacketResource::GetHash()
{
	return m_Hash;
}

void PacketResource::SetHash(Hash _hash)
{
	m_Hash = _hash;
}

void PacketResource::SetFactoryReference(PacketResourceFactory* _factoryReference)
{
	m_FactoryPtr = _factoryReference;
}

void PacketResource::SetPedingReplacement()
{
	m_IsPendingReplacement = true;
}

void PacketResource::RedirectInstancesToResource(PacketResource* _newResource)
{
#ifndef NDEBUG

	// For each instance
	for (auto* instance : m_InstancesThatUsesThisResource)
	{
		// Set the object reference
		_newResource->MakeInstanceReference(instance);

		// Reset this instance
		instance->ResetInstance();
	}

	// Clear the instance vector
	m_InstancesThatUsesThisResource.clear();

	// Zero the total number of direct references
	m_TotalDirectReferences = 0;

#endif

	// Set is pending replacement to false
	m_IsPendingReplacement = false;
}

bool PacketResource::AreInstancesReadyToBeUsed()
{
	// For each instance
	for (auto* instance : m_InstancesThatUsesThisResource)
	{
		// Check if this instance or the instance that depends on it (if there is one) are locked
		if (instance->IsLocked() || instance->InstanceDependencyIsLocked())
		{
			return false;
		}
	}

	return true;
}

void PacketResource::MakeInstanceReference(PacketResourceInstance* _instance)
{
	// Increment the total number of references
	m_TotalDirectReferences++;

	// Set the object reference
	_instance->SetObjectReference(this);


#ifndef NDEBUG

	// Insert this instance into the instance vector os dependencies
	{
		std::lock_guard<std::mutex> lock(m_InstanceVectorMutex);
		m_InstancesThatUsesThisResource.push_back(_instance);
	}

#endif
}

void PacketResource::RemoveInstanceReference(PacketResourceInstance* _instance)
{
	assert(m_TotalDirectReferences > 0);

	// Subtract one from the reference count
	m_TotalDirectReferences--;

#ifndef NDEBUG

	// Find the instance object inside the instance vector and remove it, there is no need to use
	// the mutex because this can only happens on the update phase of the PacketResourceManager
	for (unsigned int i = 0; i < m_InstancesThatUsesThisResource.size(); i++)
	{
		// Compare the pointers
		if (m_InstancesThatUsesThisResource[i] == _instance)
		{
			// Remove it
			m_InstancesThatUsesThisResource.erase(m_InstancesThatUsesThisResource.begin() + i);
		}
	}

#endif
}

uint32_t PacketResource::GetDataSize()
{
	return uint32_t(m_Data.GetSize());
}

uint32_t PacketResource::GetTotalNumberReferences()
{
	return m_TotalDirectReferences + m_TotalIndirectReferences;
}

uint32_t PacketResource::GetTotalNumberDirectReferences()
{
	return m_TotalDirectReferences;
}

uint32_t PacketResource::GetTotalNumberIndirectReferences()
{
	return m_TotalIndirectReferences;
}

PacketResourceFactory* PacketResource::GetFactoryPtr()
{
	return m_FactoryPtr;
}

/*
uint32_t& PacketResource::GetDataSizeRef()
{
	return m_Data.size();
}
*/

PacketResourceData& PacketResource::GetDataRef()
{
	return m_Data;
}