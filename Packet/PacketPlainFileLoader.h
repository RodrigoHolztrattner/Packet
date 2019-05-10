////////////////////////////////////////////////////////////////////////////////
// Filename: PacketPlainFileLoader.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
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
// Class name: PacketPlainFileLoader
////////////////////////////////////////////////////////////////////////////////
class PacketPlainFileLoader : public PacketFileLoader
{
public:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketPlainFileLoader(const PacketFileIndexer& _file_indexer);
	~PacketPlainFileLoader();

//////////////////
// MAIN METHODS //
public: //////////

    // Load a file
    std::unique_ptr<PacketFile> LoadFile(Hash _file_hash) const final;

    // Load a file raw data
    std::vector<uint8_t> LoadFileRawData(Hash _file_hash) const final;

    // Load a file references data
    std::optional<PacketFileReferences> LoadFileReferences(Hash _file_hash) const final;

///////////////
// VARIABLES //
private: //////

    // A reference to the file indexer
    const PacketFileIndexer& m_FileIndexer;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
