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
                                                                 bool _isPermanent) const
{
    // Allocate the object using the factory
    std::unique_ptr<PacketResource> resource = _resourceFactory->RequestObject();

    // Set the object hash
    resource->SetHash(_hash);

    // Set the helper pointers and the current operation mode
    resource->SetHelperObjects(_resourceFactory, m_ReferenceManagerPtr, m_FileLoaderPtr, m_LoggerPtr, m_OperationMode);

    // Set the build info
    resource->SetBuildInfo(_buildInfo);

    // Check if the file exist
#ifndef NDEBUG

    // If we shouldn't create the resource if its file doesn't exist (also if we aren't on edit mode)
    if (m_OperationMode != OperationMode::Edit || !resource->GetBuildInfo().createResourceIfInexistent)
    {
        // Check if the resource exist
        if (!m_FileLoaderPtr->FileExist(_hash))
        {
            // Error validating this resource references, we will continue but keep in mind that this resource needs to update it references hashes
            m_LoggerPtr->LogError(std::string("Trying to load a resource but it doesn't exist, path: \"")
                                  .append(_hash.GetPath().String())
                                  .append("\"")
                                  .c_str());

            assert(false);
        }
    }
#endif

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
                                    .append("\", we will continue but keep in mind that this resource needs to update it references hashes")
                                    .c_str());
        }
    }

    // If we file doesn't exist and we should create the resource in this case (also we must be on edit mode)
    if (resource->GetBuildInfo().createResourceIfInexistent && m_FileLoaderPtr->FileExist(_hash) && m_OperationMode == OperationMode::Edit)
    {
        // Call the BeginCreation() method for this object
        bool result = resource->BeginCreation(_isPermanent);
        assert(result);

        // Check if we should call the BeginLoad() method
        if (resource->GetBuildInfo().createdResourceShouldLoad)
        {
            // Call the BeginLoad() method for this object
            resource->BeginLoad(_isPermanent);
            assert(result);
        }
    }
    // Normally create the resource
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
        result = m_FileLoaderPtr->GetFileData(dataVector.GetData(), resourceSize, _hash);
        assert(result);

        // Call the BeginLoad() method for this object
        result = resource->BeginLoad(_isPermanent);
        assert(result);
    }

    return resource;
}