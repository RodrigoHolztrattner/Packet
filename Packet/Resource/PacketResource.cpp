////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResource.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResource.h"
#include "PacketResourceManager.h"

#include <cassert>
#include <fstream>

// Using namespace Peasant
PacketUsingDevelopmentNamespace(Packet)

////////////////////////
// PacketResourceData //
////////////////////////

PacketResourceData::PacketResourceData()
{
}

PacketResourceData::PacketResourceData(std::vector<uint8_t> _data)
{
    m_Data = std::move(_data);
}

const std::vector<uint8_t>& PacketResourceData::GetData() const
{
	return m_Data;
}

uint64_t PacketResourceData::GetSize() const
{
	return m_Data.size();
}

void PacketResourceData::SetData(std::vector<uint8_t>&& _data)
{
    m_Data = std::move(_data);
}

////////////////////
// PacketResource //
////////////////////

PacketResource::PacketResource()
{
	// Set the initial data
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
    // If the data size is zero we should set that the construction
    // failed directly instead trying to construct it
    if (m_Data.GetSize() == 0)
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

bool PacketResource::IsValid() const
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

void PacketResource::SetHelperObjects(
    PacketResourceFactory*  _factoryReference,
    PacketResourceManager*  _resourceManager,
    PacketLogger*           _logger,
    OperationMode           _operationMode)
{
	m_FactoryPtr = _factoryReference;
    m_ResourceManagerPtr = _resourceManager;
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

void PacketResource::SetPendingModifications()
{
    m_ResourceMutex.lock();
    m_IsPendingModifications = true;

    m_ResourceManagerPtr->RegisterResourceForModifications(this);
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

void PacketResourceCreationProxy::ForwardResourceLink(PacketResource* _resource)
{
    std::lock_guard<std::mutex> l(m_Mutex);

    // Increment the number of references for this resource
    _resource->IncrementNumberReferences();

    // If we have a valid ptr to the resource reference internal resource, update it
    if (m_ResourceReferenceVariable != nullptr)
    {
        *m_ResourceReferenceVariable = _resource;
        m_ResourceReferenceVariable = nullptr;
    }
    else
    {
        // Decrement the number of references for the resource since it won't be used
        _resource->DecrementNumberReferences();
    }
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
