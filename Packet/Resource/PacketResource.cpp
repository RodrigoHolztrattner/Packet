////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResource.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResource.h"
#include "PacketResourceInstance.h"
#include "PacketResourceFactory.h"
#include "..\PacketReferenceManager.h"

#include <cassert>
#include <fstream>

// Using namespace Peasant
PacketUsingDevelopmentNamespace(Packet)


////////////////////////
// PacketResourceData //
////////////////////////

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

uint8_t* PacketResourceData::GetData() const
{
	return m_Data;
}

uint64_t PacketResourceData::GetSize() const
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

////////////////////
// PacketResource //
////////////////////

PacketResource::PacketResource()
{
	// Set the initial data
	m_IgnorePhysicalDataChanges = false;
	m_DataValid = false;
	m_WasSynchronized = false;
	m_IsPersistent = false;
	m_IsPendingDeletion = false;
	m_WasCreated = false;
	m_HasUserFlag = false;
	m_UserFlag = false;
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

	// Set the data is valid
	m_DataValid = true;

	// Set if this object is persistent
	m_IsPersistent = _isPersistent;

	// Call the Onload() method
	if (!OnLoad(m_Data, m_BuildInfo.buildFlags, m_BuildInfo.flags))
	{
		return false;
	}

	// Set created to true
	m_WasCreated = true;

	return true;
}

bool PacketResource::BeginCreation(bool _isPersistent)
{
	// Set if this object is persistent
	m_IsPersistent = _isPersistent;

	// Call the OnCreation() method
	if (!OnCreation())
	{
		return false;
	}

	return true;
}

bool PacketResource::BeginDelete()
{
	assert(!m_WasSynchronized);

	// Check if we have some data to delete (ignore this if this resource was created)
	if (!m_WasCreated || (m_WasCreated && m_Data.GetSize() == 0))
	{
		return false;
	}

	// Set data invalid
	m_DataValid = false;

	// Call the OnDelete() method
	if (!OnDelete(m_Data))
	{
		return false;
	}

	// The data must still be valid (ignore this if this resource was created)
	if (!m_WasCreated || (m_WasCreated && m_Data.GetSize() == 0))
	{
		return false;
	}

	// Set created to false
	m_WasCreated = false;

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

bool PacketResource::BeginDesynchronization()
{
	assert(m_WasSynchronized);

	// Call the OnDesynchronization() method
	OnDesynchronization();

	// Set synchronized
	m_WasSynchronized = false;

	return true;
}

bool PacketResource::IsReady() const
{
	return (m_DataValid && m_WasSynchronized) || m_WasCreated;
}

bool PacketResource::IsPendingDeletion() const
{
	return m_IsPendingDeletion;
}

bool PacketResource::IsReferenced() const
{
	return m_TotalDirectReferences > 0 || m_TotalIndirectReferences > 0;
}

bool PacketResource::IsDirectlyReferenced() const
{
	return m_TotalDirectReferences > 0;
}

bool PacketResource::IsIndirectlyReferenced() const
{
	return m_TotalIndirectReferences > 0;
}

bool PacketResource::IsPersistent() const
{
	return m_IsPersistent;
}

void PacketResource::RegisterUserFlag()
{
	m_HasUserFlag = true;
}

void PacketResource::SetUserFlag()
{
	m_UserFlag = true;
}

bool PacketResource::HasUserFlag() const
{
	return m_HasUserFlag;
}

bool PacketResource::GetUserFlag() const
{
	return m_UserFlag;
}

Hash PacketResource::GetHash() const
{
	return m_Hash;
}

void PacketResource::SetHash(Hash _hash)
{
	m_Hash = _hash;
}

void PacketResource::SetHelperObjects(PacketResourceFactory* _factoryReference, PacketReferenceManager* _referenceManager, PacketFileLoader* _fileLoader, PacketLogger* _logger, OperationMode _operationMode)
{
	m_FactoryPtr = _factoryReference;
	m_ReferenceManagerPtr = _referenceManager;
	m_FileLoaderPtr = _fileLoader;
	m_LoggerPtr = _logger;
	m_CurrentOperationMode = _operationMode;
}

void PacketResource::SetBuildInfo(PacketResourceBuildInfo _buildInfo)
{
	m_BuildInfo = _buildInfo;
}

const PacketResourceBuildInfo& PacketResource::GetBuildInfo() const
{
	return m_BuildInfo;
}

void PacketResource::SetPendingDeletion()
{
	m_IsPendingDeletion = true;
}

void PacketResource::RedirectInstancesToResource(PacketResource* _newResource)
{
#ifndef NDEBUG

	// For each instance
	for (auto* instance : m_InstancesThatUsesThisResource)
	{
        // Lock this instance
        instance->LockUsage();

		// Set the object reference
		_newResource->MakeInstanceReference(instance);

		// Reset this instance
		instance->ResetInstance();

        // Unlock the instance
        instance->UnlockUsage();

        // Decrement the number of direct references
        m_TotalDirectReferences--;
	}

	// Clear the instance vector
	m_InstancesThatUsesThisResource.clear();

#endif
}

bool PacketResource::AreInstancesReadyToBeUsed() const
{
#ifndef NDEBUG

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

#endif

	m_LoggerPtr->LogWarning("Trying to call the method AreInstancesReadyToBeUsed() but the current build isn't a debug one!");
	return false;
}

bool PacketResource::IgnorePhysicalDataChanges() const
{
	return m_IgnorePhysicalDataChanges;
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

void PacketResource::MakeTemporaryReference()
{
	m_TotalIndirectReferences++;
}

void PacketResource::RemoveTemporaryReference()
{
	m_TotalIndirectReferences--;
}

uint32_t PacketResource::GetDataSize() const
{
	return uint32_t(m_Data.GetSize());
}

uint32_t PacketResource::GetTotalNumberReferences() const
{
	return m_TotalDirectReferences + m_TotalIndirectReferences;
}

uint32_t PacketResource::GetTotalNumberDirectReferences() const
{
	return m_TotalDirectReferences;
}

uint32_t PacketResource::GetTotalNumberIndirectReferences() const
{
	return m_TotalIndirectReferences;
}

PacketResourceFactory* PacketResource::GetFactoryPtr() const
{
	return m_FactoryPtr;
}

PacketResourceData& PacketResource::GetDataRef()
{
	return m_Data;
}

void PacketResource::IncrementNumberDirectReferences()
{
    m_TotalDirectReferences++;
}

void PacketResource::DecrementNumberDirectReferences()
{
    m_TotalDirectReferences--;
}

void PacketResource::IgnoreResourcePhysicalDataChanges()
{
	m_IgnorePhysicalDataChanges = true;
}

bool PacketResource::UpdateResourcePhysicalData(uint8_t* _data, uint64_t _dataSize)
{
	// Check the current operation mode
	if (m_CurrentOperationMode != OperationMode::Edit)
	{
		m_LoggerPtr->LogWarning("Trying to call the method UpdateResourcePhysicalData() but the operation mode is different from the Edit mode!");
		return false;
	}


#ifndef NDEBUG

	// Check the size to proceed
	if (_dataSize == 0)
	{
		return false;
	}

	// It's a little ugly to write directly to a file here but I think there is no reason to create another 
	// class just to write to a file, so I'm going with the directly write option
	{
		// Open the file
		std::ofstream file(m_Hash.GetPath().String(), std::ios::out);
		if (!file)
		{
			return false;
		}

		// Write the data into the file
		file.write((char*)_data, _dataSize);
		if (!file)
		{
			return false;
		}

		// Close the file
		file.close();
	}

	return true;

#endif

	m_LoggerPtr->LogWarning("Trying to call the method UpdateResourcePhysicalData() but the current build isn't a debug one!");
	return false;
}

bool PacketResource::UpdateResourcePhysicalData(PacketResourceData& _data)
{
	return UpdateResourcePhysicalData(_data.GetData(), _data.GetSize());
}

bool PacketResource::RegisterPhysicalResourceReference(Hash _targetResourceHash, uint64_t _hashDataLocation)
{
	// Check the current operation mode
	if (m_CurrentOperationMode != OperationMode::Edit)
	{
		m_LoggerPtr->LogWarning("Trying to call the method RegisterPhysicalResourceReference() but the operation mode is different from the Edit mode!");

		return false;
	}

#ifndef NDEBUG

	// Call the validate file references method for the reference manager
	return m_ReferenceManagerPtr->RegisterFileReference(m_Hash.GetPath().String(), _targetResourceHash.GetPath().String(), _hashDataLocation);

#endif

	m_LoggerPtr->LogWarning("Trying to call the method RegisterPhysicalResourceReference() but the current build isn't a debug one!");
	return false;
}

bool PacketResource::ClearAllPhysicalResourceReferences()
{
	// Check the current operation mode
	if (m_CurrentOperationMode != OperationMode::Edit)
	{
		m_LoggerPtr->LogWarning("Trying to call the method ClearAllPhysicalResourceReferences() but the operation mode is different from the Edit mode!");

		return false;
	}

#ifndef NDEBUG

	// Clear all file references
	m_ReferenceManagerPtr->ClearFileReferences(m_Hash.GetPath().String());

	return true;

#endif

	m_LoggerPtr->LogWarning("Trying to call the method ClearAllPhysicalResourceReferences() but the current build isn't a debug one!");
	return false;
}

bool PacketResource::VerifyPhysicalResourceReferences(ReferenceFixer _fixer, bool _allOrNothing)
{
	// Check the current operation mode
	if (m_CurrentOperationMode != OperationMode::Edit)
	{
		m_LoggerPtr->LogWarning("Trying to call the method VerifyPhysicalResourceReferences() but the operation mode is different from the Edit mode!");

		return false;
	}

#ifndef NDEBUG

	// Call the validate file references method for the reference manager
	return m_ReferenceManagerPtr->ValidateFileReferences(m_Hash.GetPath().String(), _fixer, _allOrNothing);

#endif

	m_LoggerPtr->LogWarning("Trying to call the method VerifyPhysicalResourceReferences() but the current build isn't a debug one!");
	return false;
}