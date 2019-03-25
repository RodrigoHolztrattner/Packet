////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceLoader.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "..\PacketConfig.h"

#include <thread>

///////////////
// NAMESPACE //
///////////////

/////////////
// DEFINES //
/////////////

////////////
// GLOBAL //
////////////

///////////////
// NAMESPACE //
///////////////

// Packet
PacketDevelopmentNamespaceBegin(Packet)

//////////////
// TYPEDEFS //
//////////////

////////////////
// FORWARDING //
////////////////

// Classes we know
class PacketReferenceManager;
class PacketResourceManager;
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
	PacketResourceLoader(PacketFileLoader* _fileLoaderPtr,
                         PacketReferenceManager* _referenceManager,
                         PacketResourceManager* _resourceManager,
                         PacketLogger* _loggerPtr, 
                         OperationMode _operationMode);
	~PacketResourceLoader();

//////////////////
// MAIN METHODS //
public: //////////

	// Load a new object
    std::unique_ptr<PacketResource> LoadObject(PacketResourceFactory* _resourceFactory,
                                               Hash _hash, 
                                               PacketResourceBuildInfo _buildInfo,
                                               bool _isPermanent, 
                                               bool _isRuntimeResource, 
                                               std::vector<uint8_t> _resourceData) const;

///////////////
// VARIABLES //
private: //////

	// The packet file loader ptr, the reference manager ptr, the logger ptr and the current operation mode
	PacketFileLoader*       m_FileLoaderPtr;
	PacketReferenceManager* m_ReferenceManagerPtr;
    PacketResourceManager*  m_ResourceManagerPtr;
	PacketLogger*           m_LoggerPtr;
	OperationMode           m_OperationMode;
};

// Packet
PacketDevelopmentNamespaceEnd(Packet)
