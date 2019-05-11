////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileSaver.h
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
class PacketFile;
class PacketFileHeader;
class PacketFileIndexer;
class PacketReferenceManager;
class PacketFileLoader;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFileSaver
////////////////////////////////////////////////////////////////////////////////
class PacketFileSaver
{
public:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFileSaver(const PacketFileIndexer& _file_indexer,
                    const PacketReferenceManager& _reference_manager, 
                    const PacketFileLoader& _file_loader, 
                    std::filesystem::path _packet_path);
	~PacketFileSaver();

//////////////////
// MAIN METHODS //
public: //////////

    // Save a file into disk, optionally set to not update the references
    bool SaveFile(std::unique_ptr<PacketFile> _file) const;

    // Save a part of a file data, if the given part of the file requires expansion or
    // shrinking, the entire file will be loaded to perform the change
    bool SaveFile(const PacketFileHeader& _file_header, FilePart _file_part, std::vector<uint8_t>&& _file_data_part) const;

private:

    // Save a file into disk, helper method
    bool SaveFileHelper(std::unique_ptr<PacketFile> _file) const;

///////////////
// VARIABLES //
private: //////

    // Our packet path
    std::filesystem::path m_PacketPath;

    // A reference to the file indexer, reference manager and file loader
    const PacketFileIndexer& m_FileIndexer;
    const PacketReferenceManager& m_ReferenceManager;
    const PacketFileLoader& m_FileLoader;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
