////////////////////////////////////////////////////////////////////////////////
// Filename: PacketCondensedFileLoader.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "../../PacketConfig.h"
#include "PacketFileLoader.h"

///////////////
// NAMESPACE //
///////////////

// Packet data explorer
PacketDevelopmentNamespaceBegin(Packet)

// Classes we know
class PacketFile;
class PacketFileIndexer;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketCondensedFileLoader
////////////////////////////////////////////////////////////////////////////////
class PacketCondensedFileLoader : public PacketFileLoader
{
private:

	// The condensed file reader type
	struct CondensedFileReader
	{
		// The input file object
		std::ifstream* file = nullptr;

		// The file path
		Path filePath;

		// The total number of files inside
		uint32_t totalNumberFiles;
	};

	// The mapped internal file info type
	struct MappedInternalFileInfo
	{
		// The internal file info
		CondensedFileInfo::InternalFileInfo info;

		// The reader index that we should used to read the data from this file
		uint16_t readerIndex;
	};

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketCondensedFileLoader(const PacketFileIndexer& _file_indexer, std::filesystem::path _packet_path);
	~PacketCondensedFileLoader();

//////////////////
// MAIN METHODS //
public: //////////

    // Load a file
    std::unique_ptr<PacketFile> LoadFile(Hash _file_hash) const final;

    // Load a file raw data
    std::vector<uint8_t> LoadFileRawData(Hash _file_hash) const final;

    // Load a file data part
    std::optional<std::tuple<PacketFileHeader, std::vector<uint8_t>>> LoadFileDataPart(Hash _file_hash, FilePart _file_part) const final;

///////////////
// VARIABLES //
private: //////

};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
