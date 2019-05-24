////////////////////////////////////////////////////////////////////////////////
// Filename: PacketBackupManager.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "../../PacketConfig.h"

///////////////
// NAMESPACE //
///////////////

// Packet data explorer
PacketDevelopmentNamespaceBegin(Packet)

// Classes we know
class PacketFileSaver;
class PacketFileManager;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketBackupManager
////////////////////////////////////////////////////////////////////////////////
class PacketBackupManager
{
    // Friend classes
    friend PacketFileSaver;
    friend PacketFileManager;

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketBackupManager(std::filesystem::path _packet_path, std::filesystem::path _backup_path);
	~PacketBackupManager();

//////////////////
// MAIN METHODS //
public: //////////

    // Restore a file
    bool RestoreFile(Path _file_path) const;

protected:

    // Backup a file
    bool BackupFile(Path _file_path) const;

///////////////
// VARIABLES //
private: //////

    // Our packet path
    std::filesystem::path m_PacketPath;
    std::filesystem::path m_BackupPath;

    // If this object is active (if it should perform backups and restore operations)
    bool m_IsActive = false;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
