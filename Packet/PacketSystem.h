////////////////////////////////////////////////////////////////////////////////
// Filename: PacketSystem.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "Resource/PacketResourceManager.h"

///////////////
// NAMESPACE //
///////////////

// Packet data explorer
PacketDevelopmentNamespaceBegin(Packet)

// Classes we know
class PacketFileManager;
class PacketResourceFactory;
class PacketResourceWatcher;
class PacketResourceStorage;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketSystem
////////////////////////////////////////////////////////////////////////////////
class PacketSystem
{
public:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketSystem();
	~PacketSystem();

//////////////////
// MAIN METHODS //
public: //////////

	// Initialize this object
	bool Initialize(OperationMode _operation_mode, std::filesystem::path _resource_path, std::unique_ptr<PacketLogger>&& _logger = std::make_unique<PacketLogger>());
    
    // Return the resource (packet) path
    std::filesystem::path GetResourcePath() const;

    // Return a reference to our objects
    PacketFileManager& GetFileManager();
    PacketResourceManager& GetResourceManager();

///////////////
// VARIABLES //
private: //////

    // The resource path and operation mode
    std::filesystem::path m_ResourcePath;
    OperationMode m_OperationMode = OperationMode::Plain;

	// Our internal objects
    std::unique_ptr<PacketFileManager>      m_FileManager;
    std::unique_ptr<PacketReferenceManager> m_ReferenceManager;
    std::unique_ptr<PacketResourceStorage>  m_ResourceStorage;
    std::unique_ptr<PacketResourceManager>  m_ResourceManager;
    std::unique_ptr<PacketResourceWatcher>  m_ResourceWatcher;
    std::unique_ptr<PacketLogger>           m_Logger;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
