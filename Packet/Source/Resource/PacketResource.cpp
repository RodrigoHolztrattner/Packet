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
	m_TotalReferences = 0;
}

PacketResource::~PacketResource()
{
	assert(m_TotalReferences == 0);
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

bool PacketResource::IsReferenced()
{
	return m_TotalReferences > 0;
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

void PacketResource::MakeInstanceReference(PacketResourceInstance* _instance)
{
	// Increment the total number of references
	m_TotalReferences++;

	// Set the object reference
	_instance->SetObjectReference(this);
}

void PacketResource::MakeInstanceReference()
{
	// Increment the total number of references
	m_TotalReferences++;
}

void PacketResource::ReleaseInstance()
{
	assert(m_TotalReferences > 0);

	// Subtract one from the reference count
	m_TotalReferences--;
}

uint32_t PacketResource::GetDataSize()
{
	return uint32_t(m_Data.GetSize());
}

uint32_t PacketResource::GetTotalNumberReferences()
{
	return m_TotalReferences;
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