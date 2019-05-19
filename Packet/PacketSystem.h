////////////////////////////////////////////////////////////////////////////////
// Filename: PacketSystem.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"

///////////////
// NAMESPACE //
///////////////

// Packet data explorer
PacketDevelopmentNamespaceBegin(Packet)

// Classes we know
class PacketFileManager;
class PacketResourceFactory;
class PacketResourceStorage;
class PacketResourceManager;

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

	// Initialize this object (usually for condensed mode)
	bool Initialize(OperationMode                   _operation_mode, 
                    std::filesystem::path           _resource_path, 
                    std::unique_ptr<PacketLogger>&& _logger = std::make_unique<PacketLogger>());

    // Initialize this object (usually for plain mode)
    bool Initialize(OperationMode                   _operation_mode,
                    BackupFlags                     _backup_flags,
                    std::filesystem::path           _resource_path,
                    std::filesystem::path           _backup_path,
                    std::unique_ptr<PacketLogger>&& _logger = std::make_unique<PacketLogger>());

    // Return the resource (packet) path
    std::filesystem::path GetResourcePath() const;

    // Return a reference to our objects
    PacketFileManager& GetFileManager();
    PacketResourceManager& GetResourceManager();

///////////////
// VARIABLES //
private: //////

    // The resource path, backup path, operation mode and backup flags
    std::filesystem::path m_ResourcePath;
    std::filesystem::path m_BackupPath;
    OperationMode m_OperationMode = OperationMode::Plain;
    BackupFlags m_BackupFlags     = BackupFlags::None;

	// Our internal objects
    std::unique_ptr<PacketFileManager>      m_FileManager;
    std::unique_ptr<PacketResourceStorage>  m_ResourceStorage;
    std::unique_ptr<PacketResourceManager>  m_ResourceManager;
    std::unique_ptr<PacketLogger>           m_Logger;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
