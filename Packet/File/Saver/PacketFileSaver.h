////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileSaver.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "../../PacketConfig.h"
#include <set>

///////////////
// NAMESPACE //
///////////////

// Packet data explorer
PacketDevelopmentNamespaceBegin(Packet)

// Classes we know
class PacketFile;
class PacketFileHeader;
class PacketFileIndexer;
class PacketReferenceManager;
class PacketFileLoader;
class PacketFileManager;
class PacketBackupManager;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFileSaver
////////////////////////////////////////////////////////////////////////////////
class PacketFileSaver
{
public:

    // Friend classes
    friend PacketFileManager;

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFileSaver(const PacketFileIndexer& _file_indexer,
                    const PacketReferenceManager& _reference_manager, 
                    const PacketFileLoader& _file_loader, 
                    const PacketBackupManager& _backup_manager,
                    std::filesystem::path _packet_path, 
                    bool _backup_before_saving);
	~PacketFileSaver();

//////////////////
// MAIN METHODS //
public: //////////

    // Save a file into disk, optionally set to not update the references
    bool SaveFile(std::unique_ptr<PacketFile> _file, SaveOperation _operation) const;

    // Save a part of a file data, if the given part of the file requires expansion or
    // shrinking, the entire file will be loaded to perform the change
    // This operation is considered a overwrite operation since the file already exist
    bool SaveFile(const PacketFileHeader& _file_header, FilePart _file_part, std::vector<uint8_t>&& _file_data_part) const;

///////////////////////////////
protected: // AFFECTED FILES //
///////////////////////////////

    // Clear the current affected file set
    void ClearAffectedFiles();

    // Return the affected file set
    const std::set<Path> GetAffectedFiles() const;

/////////////////////////////
private: // HELPER METHODS //
/////////////////////////////

    // Save a file into disk, helper method
    bool SaveFileHelper(std::unique_ptr<PacketFile> _file) const;

///////////////
// VARIABLES //
private: //////

    // Our packet path
    std::filesystem::path m_PacketPath;

    // A reference to the file indexer, reference manager, file loader and backup manager
    const PacketFileIndexer&      m_FileIndexer;
    const PacketReferenceManager& m_ReferenceManager;
    const PacketFileLoader&       m_FileLoader;
    const PacketBackupManager&    m_BackupManager;

    // If this is true we must issue a backup before modifying a file
    bool m_BackupBeforeSaving = false;

    // This is a set of files that were modified by this saver object, they
    // can be used to determine all affected files in case an operation
    // fails and their backups must be restored
    mutable std::set<Path> m_AffectedFiles;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
