////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceLoader.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResourceLoader.h"
#include "PacketResourceFactory.h"
#include "PacketResourceManager.h"
#include "PacketResource.h"
#include "../File/PacketFile.h"

#include <cassert>
#include <chrono>

// Using namespace Peasant
PacketUsingDevelopmentNamespace(Packet)

PacketResourceLoader::PacketResourceLoader(
    PacketFileLoader&       _fileLoader,
    PacketResourceManager&  _resourceManager,
    PacketLogger*           _loggerPtr,
    OperationMode           _operationMode) :
    m_FileLoader(_fileLoader), 
    m_ResourceManager(_resourceManager), 
    m_LoggerPtr(_loggerPtr),
    m_OperationMode(_operationMode)
{
}

PacketResourceLoader::~PacketResourceLoader()
{
}

std::unique_ptr<PacketResource> PacketResourceLoader::LoadObject(PacketResourceFactory* _resourceFactory,
                                                                 Hash _hash, 
                                                                 PacketResourceBuildInfo _buildInfo,
                                                                 bool _isPermanent, 
                                                                 bool _isRuntimeResource,
                                                                 std::vector<uint8_t> _resourceData) const
{
    // Allocate the object using the factory
    std::unique_ptr<PacketResource> resource = _resourceFactory->RequestObject();

    // Set the object hash
    resource->SetHash(_hash);

    // Set the helper pointers and the current operation mode
    resource->SetHelperObjects(
        _resourceFactory,
        &m_ResourceManager,
        &m_FileLoader,
        m_LoggerPtr,
        m_OperationMode);

    // Set the build info
    resource->SetBuildInfo(_buildInfo, _isRuntimeResource);

    // Check if this is a runtime resource and the data shouldn't come from a file
    if (_isRuntimeResource)
    {
        // Get a reference to the object data vector directly
        auto& dataVector = resource->GetDataRef();
        dataVector = PacketResourceData(std::move(_resourceData));

        // Call the BeginLoad() method for this object
        bool result = resource->BeginLoad(_isPermanent);
        assert(result);
    }
    else
    {
        // Get the resource size
        auto resourceSize = m_FileLoader.GetFileSize(_hash);

        // Get a reference to the object data vector directly
        auto& dataVector = resource->GetDataRef();

        // Using the object factory, allocate the necessary data
        bool result = resource->GetFactoryPtr()->AllocateData(dataVector, resourceSize);
        assert(result);

        // Read the file data
        result = m_FileLoader.GetFileData(dataVector.GetwritableData(), resourceSize, _hash);
        assert(result);

        // Call the BeginLoad() method for this object
        result = resource->BeginLoad(_isPermanent);
        assert(result);
    }

    // Begin the construction for this resource
    resource->BeginConstruct();

    // If this resource doesn't need external construct, call it here to set the internal flags, else
    // the manager will make all necessary adjustments to externally synchronize it
    if (!resource->RequiresExternalConstructPhase() && !resource->ConstructionFailed())
    {
        resource->BeginExternalConstruct(nullptr);
    }

    return resource;
}