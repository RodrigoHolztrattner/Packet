////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceLoader.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResourceLoader.h"
#include "../Factory/PacketResourceFactory.h"
#include "../PacketResourceManager.h"
#include "../PacketResource.h"
#include "../../File/PacketFile.h"
#include "../../File/Loader/PacketFileLoader.h"

#include <cassert>
#include <chrono>

// Using namespace Peasant
PacketUsingDevelopmentNamespace(Packet)

PacketResourceLoader::PacketResourceLoader(
    const PacketFileLoader& _fileLoader,
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
                                                                 std::vector<uint8_t> _resourceData) const
{
    // Load the file
    auto resource_file = m_FileLoader.LoadFile(_hash);
    assert(resource_file);

    // Retrieve the file final data
    std::vector<uint8_t> file_final_data = PacketFile::RetrieveFileFinalData(std::move(resource_file));

    // Allocate the object using the factory
    std::unique_ptr<PacketResource> resource = _resourceFactory->RequestObject(file_final_data);

    // Set the object hash
    // Set the build info
    // Set the helper pointers and the current operation mode
    resource->SetHash(_hash);
    resource->SetBuildInfo(_buildInfo);
    resource->SetHelperObjects(
        _resourceFactory,
        &m_ResourceManager,
        m_LoggerPtr,
        m_OperationMode);

    // Get a reference to the object data vector directly
    auto& data_vector = resource->GetDataRef();

    // Set the data
    data_vector.SetData(std::move(file_final_data));

    // Call the BeginLoad() method for this object
    bool result = resource->BeginLoad(_isPermanent);
    assert(result);

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