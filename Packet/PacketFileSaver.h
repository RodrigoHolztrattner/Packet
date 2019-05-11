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
	PacketFileSaver(const PacketFileIndexer& _file_indexer, std::filesystem::path _packet_path);
	~PacketFileSaver();

//////////////////
// MAIN METHODS //
public: //////////

    // Save a file into disk
    bool SaveFile(std::unique_ptr<PacketFile> _file) const;

    // Save a file by its raw data
    bool SaveFile(std::vector<uint8_t>&& _file_raw_data) const;

    // Save a part of a file data, if the given part of the file requires expansion or
    // shrinking, the entire file will be loaded to perform the change
    bool SaveFile(const PacketFileHeader& _file_header, FilePart _file_part, std::vector<uint8_t>&& _file_data_part) const;

///////////////
// VARIABLES //
private: //////

    // Our packet path
    std::filesystem::path m_PacketPath;

    // A reference to the file indexer
    const PacketFileIndexer& m_FileIndexer;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
