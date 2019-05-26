////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileLoader.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "../../PacketConfig.h"
#include "../PacketFileHeader.h"
#include "../PacketFile.h"

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

// Classes we know
class PacketFile;
class PacketReferenceManager;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFileLoader
////////////////////////////////////////////////////////////////////////////////
class PacketFileLoader
{
public:

    // Friend classes
    friend PacketReferenceManager;

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFileLoader(std::filesystem::path _packet_path);
    virtual ~PacketFileLoader();

//////////////////
// MAIN METHODS //
public: //////////

    // Load a file
    virtual std::unique_ptr<PacketFile> LoadFile(HashPrimitive _file_hash) const = 0;

    // Load a file raw data
    virtual std::vector<uint8_t> LoadFileRawData(HashPrimitive _file_hash) const = 0;

    // Load a file data part
    virtual std::optional<std::tuple<PacketFileHeader, std::vector<uint8_t>>> LoadFileDataPart(HashPrimitive _file_hash, FilePart _file_part) const = 0;

///////////////
// VARIABLES //
protected: ////

    // Our packet path
    std::filesystem::path m_PacketPath;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
