////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceLoader.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResourceLoader.h"
#include "PacketResourceFactory.h"
#include "PacketResource.h"
#include "..\PacketReferenceManager.h"

#include <cassert>
#include <chrono>

// Using namespace Peasant
PacketUsingDevelopmentNamespace(Packet)

PacketResourceLoader::PacketResourceLoader(PacketFileLoader* _fileLoaderPtr,
                                           PacketReferenceManager* _referenceManager,
                                           PacketLogger* _loggerPtr, 
                                           OperationMode _operationMode)
{
	// Save the pointers and the operation mode
	m_FileLoaderPtr = _fileLoaderPtr;
	m_ReferenceManagerPtr = _referenceManager;
	m_LoggerPtr = _loggerPtr;
	m_OperationMode = _operationMode;
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
    resource->SetHelperObjects(_resourceFactory, m_ReferenceManagerPtr, m_FileLoaderPtr, m_LoggerPtr, m_OperationMode);

    // Set the build info
    resource->SetBuildInfo(_buildInfo, _isRuntimeResource);

    // Check the operation mode to know if we should validate this resource references
    if (m_OperationMode == OperationMode::Edit)
    {
        // Validate this resource file references (if they exist), also try to fix them if they are invalid
        bool validationResult = m_ReferenceManagerPtr->ValidateFileReferences(_hash.GetPath().String(), ReferenceFixer::AtLeastNameAndExtension);
        if (!validationResult)
        {
            // Error validating this resource references, we will continue but keep in mind that this resource needs to update it references hashes
            m_LoggerPtr->LogWarning(std::string("Found an error when validating a resource references (\"")
                                    .append(_hash.GetPath().String())
                                    .append("\", we will continue but keep in mind that this resource needs to update its references hashes")
                                    .c_str());
        }
    }

    // Check if this is a runtime resource and the data shouldn't come from a file
    if (_isRuntimeResource)
    {
        // Get the resource size
        auto resourceSize = _resourceData.size();

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
        auto resourceSize = m_FileLoaderPtr->GetFileSize(_hash);

        // Get a reference to the object data vector directly
        auto& dataVector = resource->GetDataRef();

        // Using the object factory, allocate the necessary data
        bool result = resource->GetFactoryPtr()->AllocateData(dataVector, resourceSize);
        assert(result);

        // Read the file data
        result = m_FileLoaderPtr->GetFileData(dataVector.GetwritableData(), resourceSize, _hash);
        assert(result);

        // Call the BeginLoad() method for this object
        result = resource->BeginLoad(_isPermanent);
        assert(result);
    }

    return resource;
}