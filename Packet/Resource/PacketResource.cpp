////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResource.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResource.h"
#include "PacketResourceManager.h"
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

PacketResourceData::PacketResourceData(uint64_t _size) : m_Data(nullptr)
{
	// Allocate the initial memory if the size isn't 0
    if (_size != 0)
    {
        bool result = AllocateMemory(_size);
        assert(result);
    }
}

PacketResourceData::PacketResourceData(std::vector<uint8_t> _runtimeData) : m_Data(nullptr), m_Size(0)
{
    m_RuntimeData = std::move(_runtimeData);
}

PacketResourceData::PacketResourceData(PacketResourceData& _other)
{
    // Deallocate the current memory
    DeallocateMemory();

    // Allocate the necessary memory if the size isn't 0
    if (_other.m_Size)
    {
        AllocateMemory(_other.m_Size);
    } 

	m_Data = _other.m_Data;
	m_Size = _other.m_Size;
    m_RuntimeData = _other.m_RuntimeData;
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
        // Deallocate the current memory
        DeallocateMemory();

        // Allocate the necessary memory if the size isn't 0
        if (_other.m_Size)
        {
            AllocateMemory(_other.m_Size);
        }
        
		// Acquire the other resource instance ptr
        memcpy(m_Data, _other.m_Data, _other.m_Size);
		m_Size = _other.m_Size;
        m_RuntimeData = _other.m_RuntimeData;
	}

	return *this;
}

PacketResourceData::PacketResourceData(PacketResourceData&& _other)
{
	// Acquire the other resource instance ptr
    m_Data = _other.m_Data;
    m_Size = _other.m_Size;
    m_RuntimeData = std::move(_other.m_RuntimeData);
    _other.m_Data = nullptr;
    _other.m_Size = 0;
}

const uint8_t* PacketResourceData::GetData() const
{
    if (m_RuntimeData.size() > 0)
    {
        return static_cast<const uint8_t*>(m_RuntimeData.data());
    }

	return m_Data;
}

uint8_t* PacketResourceData::GetwritableData() const
{
    return m_Data;
}

uint64_t PacketResourceData::GetSize() const
{
    if (m_RuntimeData.size() > 0)
    {
        return m_RuntimeData.size();
    }

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
    if (m_Data != nullptr)
    {
        delete[] m_Data;
        m_Data = nullptr;
        m_Size = 0;
    }
}

////////////////////
// PacketResource //
////////////////////

PacketResource::PacketResource()
{
	// Set the initial data
	m_IgnorePhysicalDataChanges = false;
	m_IsPermanentResource       = false;
	m_IsPendingDeletion         = false;
    m_TotalReferences           = 0;
}

PacketResource::~PacketResource()
{
	assert(m_TotalReferences == 0);
}

bool PacketResource::BeginLoad(bool _isPersistent)
{
	// Set if this object is persistent
	m_IsPermanentResource = _isPersistent;

	// Set loaded to true
    m_WasLoaded = true;

	return true;
}

bool PacketResource::BeginDelete()
{
	// Call the OnDelete() method
	if (!OnDelete(m_Data))
	{
		return false;
	}

	// Set loaded and constructed to false since this resource isn't valid anymore
    m_WasLoaded = false;
    m_WasConstructed = false;
    m_WasExternallyConstructed = false;

	return true;
}

void PacketResource::BeginConstruct()
{
    // If the data size is zero and this is not a runtime resource we should set that the construction
    // failed directly instead trying to construct it
    if (m_Data.GetSize() == 0 && !m_IsRuntimeResource)
    {
        m_ConstructFailed = true;
    }
    else
    {
        bool result = OnConstruct(m_Data, m_BuildInfo.buildFlags, m_BuildInfo.flags);
        m_ConstructFailed = !result;

        m_WasConstructed = true;
    }
}

void PacketResource::BeginExternalConstruct(void* _data)
{
    bool result = OnExternalConstruct(m_Data, m_BuildInfo.buildFlags, m_BuildInfo.flags, _data);
    m_ConstructFailed = m_ConstructFailed && !result;

    m_WasExternallyConstructed = true;
}

void PacketResource::BeginModifications()
{
    bool result = OnModification();
    if (!result)
    {
        // TODO:
        // Revert changes?! (Is it possible to revert the changes successfully?)
    }

    m_IsPendingModifications = false;
    m_ResourceMutex.unlock();
}

bool PacketResource::IsReady() const
{
	return m_WasLoaded
        && m_WasConstructed
        && m_WasExternallyConstructed
        && !m_IsPendingModifications
        && !m_IsPendingDeletion
        && !m_ConstructFailed;
}

bool PacketResource::IsPendingDeletion() const
{
	return m_IsPendingDeletion;
}

bool PacketResource::IsReferenced() const
{
	return m_TotalReferences > 0;
}

bool PacketResource::IsPermanent() const
{
	return m_IsPermanentResource;
}

bool PacketResource::IsRuntime() const
{
    return m_IsRuntimeResource;
}

bool PacketResource::IsPendingModifications() const
{
    return m_IsPendingModifications;
}

bool PacketResource::RequiresExternalConstructPhase() const
{ 
    return false; 
}

bool PacketResource::OnConstruct(PacketResourceData&, uint32_t, uint32_t)
{
    return true;
}

bool PacketResource::OnExternalConstruct(PacketResourceData&, uint32_t, uint32_t, void*)
{
    return true;
}

bool PacketResource::OnModification()
{
    return true;
}

Hash PacketResource::GetHash() const
{
	return m_Hash;
}

void PacketResource::SetHash(Hash _hash)
{
	m_Hash = _hash;
}

void PacketResource::SetHelperObjects(PacketResourceFactory* _factoryReference,
                                      PacketReferenceManager* _referenceManager, 
                                      PacketResourceManager* _resourceManager,
                                      PacketFileLoader* _fileLoader, 
                                      PacketLogger* _logger,
                                      OperationMode _operationMode)
{
	m_FactoryPtr = _factoryReference;
	m_ReferenceManagerPtr = _referenceManager;
    m_ResourceManagerPtr = _resourceManager;
	m_FileLoaderPtr = _fileLoader;
	m_LoggerPtr = _logger;
	m_CurrentOperationMode = _operationMode;
}

void PacketResource::SetBuildInfo(PacketResourceBuildInfo _buildInfo,
                                  bool _isRuntimeResource)
{
	m_BuildInfo = _buildInfo;
    m_IsRuntimeResource = _isRuntimeResource;
}

const PacketResourceBuildInfo& PacketResource::GetBuildInfo() const
{
	return m_BuildInfo;
}

void PacketResource::SetPendingDeletion()
{
	m_IsPendingDeletion = true;
}

void PacketResource::SetPendingModifications()
{
    m_ResourceMutex.lock();
    m_IsPendingModifications = true;

    m_ResourceManagerPtr->RegisterResourceForModifications(this);
}

bool PacketResource::IgnorePhysicalDataChanges() const
{
	return m_IgnorePhysicalDataChanges;
}

bool PacketResource::ConstructionFailed() const
{
    return m_ConstructFailed;
}

void PacketResource::IncrementNumberReferences()
{
    m_TotalReferences++;
}

void PacketResource::DecrementNumberReferences(bool _releaseEnable)
{
    bool releaseResource = false;

    {
        std::lock_guard<std::mutex> l(m_ResourceMutex);

        // Do not release the resource if it is being replaced by another, if that's the case this resource
        // is already in the deletion queue for the resource manager object
        releaseResource = (m_TotalReferences == 1) && m_ReplacingResource == nullptr && _releaseEnable;

        m_TotalReferences--;
    }

    if (releaseResource)
    {
        // Use the resource manager to release this instance
        m_ResourceManagerPtr->RegisterResourceForDeletion(this);
    }
}

uint32_t PacketResource::GetDataSize() const
{
	return uint32_t(m_Data.GetSize());
}

uint32_t PacketResource::GetTotalNumberReferences() const
{
	return m_TotalReferences;
}

PacketResourceFactory* PacketResource::GetFactoryPtr() const
{
	return m_FactoryPtr;
}

OperationMode PacketResource::GetOperationMode() const
{
    return m_CurrentOperationMode;
}

PacketResourceData& PacketResource::GetDataRef()
{
	return m_Data;
}

void PacketResource::IgnoreResourcePhysicalDataChanges()
{
	m_IgnorePhysicalDataChanges = true;
}

bool PacketResource::UpdateResourcePhysicalData(const uint8_t* _data, uint64_t _dataSize)
{
	// Check the current operation mode
	if (m_CurrentOperationMode != OperationMode::Edit)
	{
		m_LoggerPtr->LogWarning("Trying to call the method UpdateResourcePhysicalData() but the operation mode is different from the Edit mode!");
		return false;
	}

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

	// Call the validate file references method for the reference manager
	return m_ReferenceManagerPtr->RegisterFileReference(m_Hash.GetPath().String(), _targetResourceHash.GetPath().String(), _hashDataLocation);
}

bool PacketResource::ClearAllPhysicalResourceReferences()
{
	// Check the current operation mode
	if (m_CurrentOperationMode != OperationMode::Edit)
	{
		m_LoggerPtr->LogWarning("Trying to call the method ClearAllPhysicalResourceReferences() but the operation mode is different from the Edit mode!");

		return false;
	}

	// Clear all file references
	m_ReferenceManagerPtr->ClearFileReferences(m_Hash.GetPath().String());

	return true;
}

bool PacketResource::VerifyPhysicalResourceReferences(ReferenceFixer _fixer, bool _allOrNothing)
{
	// Check the current operation mode
	if (m_CurrentOperationMode != OperationMode::Edit)
	{
		m_LoggerPtr->LogWarning("Trying to call the method VerifyPhysicalResourceReferences() but the operation mode is different from the Edit mode!");

		return false;
	}

	// Call the validate file references method for the reference manager
	return m_ReferenceManagerPtr->ValidateFileReferences(m_Hash.GetPath().String(), _fixer, _allOrNothing);
}

void PacketResource::RegisterReplacingResource(PacketResource* _replacingObject)
{
    m_ReplacingResource = _replacingObject;
}


std::optional<PacketResource*> PacketResource::GetReplacingResource() const
{
    if (m_ReplacingResource == nullptr)
    {
        return std::nullopt;
    }

    return m_ReplacingResource;
}

/////////////////////////////////
// PacketResourceCreationProxy //
/////////////////////////////////

void PacketResourceCreationProxy::LinkWithProxyInterface(PacketResourceCreationProxyInterface* _proxyInterface)
{
    std::lock_guard<std::mutex> l(m_Mutex);
    assert(m_ProxyInterfacePtr == nullptr);
    assert(m_IsLinkedWithReference == false);

    m_ProxyInterfacePtr = _proxyInterface;
    m_IsLinkedWithReference = true;
}

void PacketResourceCreationProxy::UnlinkFromProxy()
{
    std::lock_guard<std::mutex> l(m_Mutex);
    assert(m_ProxyInterfacePtr != nullptr);
    assert(m_IsLinkedWithReference == true);

    m_IsLinkedWithReference = false;
    m_ProxyInterfacePtr = nullptr;
}

void PacketResourceCreationProxy::ForwardResourceLink(PacketResource * _resource)
{
    std::lock_guard<std::mutex> l(m_Mutex);

    if (m_IsLinkedWithReference)
    {
        m_ProxyInterfacePtr->ForwardResourceLink(_resource);
        m_ProxyInterfacePtr = nullptr;
        m_IsLinkedWithReference = false;
    }
    else
    {
        // Decrement the resource ref count
        _resource->DecrementNumberReferences();
    }
}

//////////////////////////////////////////
// PacketResourceCreationProxyInterface //
//////////////////////////////////////////

PacketResourceCreationProxyInterface::~PacketResourceCreationProxyInterface()
{
    if (m_CreationProxy != nullptr)
    {
        m_CreationProxy->UnlinkFromProxy();
    }
}

PacketResourceCreationProxyInterface::PacketResourceCreationProxyInterface(const PacketResourceCreationProxyInterface& _other)
{
    m_CreationProxy = nullptr;
    m_PendingResourceVariable = nullptr;
};

PacketResourceCreationProxyInterface& PacketResourceCreationProxyInterface::operator=(const PacketResourceCreationProxyInterface& _other)
{
    m_CreationProxy = nullptr;
    m_PendingResourceVariable = nullptr;

    return *this;
};

PacketResourceCreationProxyInterface& PacketResourceCreationProxyInterface::operator=(PacketResourceCreationProxyInterface&& _other)
{
    m_CreationProxy = std::move(_other.m_CreationProxy);
    m_PendingResourceVariable = std::move(_other.m_PendingResourceVariable);
    _other.m_CreationProxy = nullptr;
    _other.m_PendingResourceVariable = nullptr;

    return *this;
}

PacketResourceCreationProxyInterface::PacketResourceCreationProxyInterface(PacketResourceCreationProxyInterface&& _other)
{
    m_CreationProxy = std::move(_other.m_CreationProxy);
    m_PendingResourceVariable = std::move(_other.m_PendingResourceVariable);
    _other.m_CreationProxy = nullptr;
    _other.m_PendingResourceVariable = nullptr;
}

void PacketResourceCreationProxyInterface::SetProxyAndResource(PacketResourceCreationProxy* _creationProxy, PacketResource** _resourceVariable)
{
    m_CreationProxy = _creationProxy;
    m_CreationProxy->LinkWithProxyInterface(this);

    m_PendingResourceVariable = _resourceVariable;
}

void PacketResourceCreationProxyInterface::ForwardResourceLink(PacketResource* _resource)
{
    m_CreationProxy = nullptr;
    *m_PendingResourceVariable = _resource;

    // Increment the resource ref count
    _resource->IncrementNumberReferences();
}

///////////////////////////////////////
// PacketResourceExternalConstructor //
///////////////////////////////////////

PacketResourceExternalConstructor::PacketResourceExternalConstructor(PacketResource* _resource, PacketResourceManager* _manager) :
    m_Resource(_resource),
    m_ResourceManager(_manager)
{
}

PacketResourceExternalConstructor::PacketResourceExternalConstructor() :
    m_Resource(nullptr),
    m_ResourceManager(nullptr)
{
}

PacketResourceExternalConstructor::PacketResourceExternalConstructor(PacketResourceExternalConstructor&& _other)
{
    m_ResourceManager = std::move(_other.m_ResourceManager);
    m_Resource = std::move(_other.m_Resource);
    _other.m_ResourceManager = nullptr;
    _other.m_Resource = nullptr;
}

PacketResourceExternalConstructor& PacketResourceExternalConstructor::operator=(PacketResourceExternalConstructor&& _other)
{
    if (this != &_other)
    {
        m_ResourceManager = std::move(_other.m_ResourceManager);
        m_Resource = std::move(_other.m_Resource);
        _other.m_ResourceManager = nullptr;
        _other.m_Resource = nullptr;
    }

    return *this;
}

PacketResourceExternalConstructor::~PacketResourceExternalConstructor()
{
    if (m_Resource != nullptr)
    {
        // We need to reinsert this resource into the manager queue because it wasn't 
        // constructed
        m_ResourceManager->RegisterResourceForExternalConstruction(m_Resource);
    }
}

void PacketResourceExternalConstructor::ConstructResource(void* _data)
{
    if (m_Resource != nullptr)
    {
        m_Resource->BeginExternalConstruct(_data);
    }

    // The resource will be automatically ready, we don't need to do any other processing
    // and our internal ptr can be set to null
    m_Resource = nullptr;
}
