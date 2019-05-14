////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceLoader.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "../../PacketConfig.h"

///////////////
// NAMESPACE //
///////////////

// Packet
PacketDevelopmentNamespaceBegin(Packet)

// Classes we know
class PacketFileLoader;
class PacketResourceManager;
class PacketLogger;
class PacketResource;
class PacketResourceFactory;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketResourceLoader
////////////////////////////////////////////////////////////////////////////////
class PacketResourceLoader
{

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
    PacketResourceLoader(
        const PacketFileLoader& _fileLoader,
        PacketResourceManager&  _resourceManager,
        PacketLogger*           _loggerPtr,
        OperationMode           _operationMode);
	~PacketResourceLoader();

//////////////////
// MAIN METHODS //
public: //////////

	// Load a new object
    std::unique_ptr<PacketResource> LoadObject(
        PacketResourceFactory*  _resourceFactory,
        Hash                    _hash,
        PacketResourceBuildInfo _buildInfo,
        bool                    _isPermanent,
        bool                    _isRuntimeResource,
        std::vector<uint8_t>    _resourceData) const;

///////////////
// VARIABLES //
private: //////

	// The packet file loader ptr, the reference manager ptr, the logger ptr and the current operation mode
    const PacketFileLoader& m_FileLoader;
    PacketResourceManager&  m_ResourceManager;
	PacketLogger*           m_LoggerPtr;
	OperationMode           m_OperationMode;
};

// Packet
PacketDevelopmentNamespaceEnd(Packet)
