////////////////////////////////////////////////////////////////////////////////
// Filename: PacketCondensedModeFileLoader.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"

#include <string>
#include <vector>
#include <map>
#include <fstream>

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

// Packet data explorer
PacketDevelopmentNamespaceBegin(Packet)

////////////////
// FORWARDING //
////////////////

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketCondensedModeFileLoader
////////////////////////////////////////////////////////////////////////////////
class PacketCondensedModeFileLoader : public PacketFileLoader
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
	PacketCondensedModeFileLoader(std::string _packetManifestDirectory);
	~PacketCondensedModeFileLoader();

//////////////////
// MAIN METHODS //
public: //////////

	// Check if a given file exist
	bool FileExist(Hash _fileHash) override;

	// Return a file size
	uint64_t GetFileSize(Hash _fileHash) override;

	// Get the file data
	bool GetFileData(uint8_t* _dataOut, uint32_t _bufferSize, Hash _fileHash) override;

	// Pack all files
	bool ConstructPacket() override;

private:

	// Read the packet data
	bool ReadPacketData(std::string _packetManifestDirectory);

	// Process the packet data
	void ProcessPacketData();

	// This method will return the hashes of the files that needs to be updated or doesn't exist for the input packed path, the 
	// date created, version and file write dates will be considered when generating this list, this is a costly method and should 
	// be used with caution
	std::vector<Hash> GetFileHashesThatNeedUpdate(std::string _packetManifestDirectory);

///////////////
// VARIABLES //
private: //////

	// If the packed data was loaded successfully
	bool m_PackedDataLoaded;

	// The condensed header
	CondensedHeaderInfo m_CondensedFileHeader;

	// All the condensed file infos
	std::vector<CondensedFileInfo> m_CondensedFileInfos;

	// Our condensed file readers
	std::vector<CondensedFileReader> m_FileReaders;

	// Our internal file info map
	std::map<HashPrimitive, MappedInternalFileInfo> m_MappedInternalFileInfos;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
