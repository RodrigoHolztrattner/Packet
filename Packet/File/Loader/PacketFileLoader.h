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
	PacketFileLoader();
	~PacketFileLoader();

//////////////////
// MAIN METHODS //
public: //////////

    // Load a file
    virtual std::unique_ptr<PacketFile> LoadFile(Hash _file_hash) const = 0;

    // Load a file raw data
    virtual std::vector<uint8_t> LoadFileRawData(Hash _file_hash) const = 0;

    // Load a file data part
    virtual std::optional<std::tuple<PacketFileHeader, std::vector<uint8_t>>> LoadFileDataPart(Hash _file_hash, FilePart _file_part) const = 0;

///////////////
// VARIABLES //
private: //////

};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
