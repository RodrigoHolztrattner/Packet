////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileIndexer.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "File/PacketFileHeader.h"
#include "File/PacketFileReferences.h"
#include "File/PacketFile.h"

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

// Classes we know
class PacketResourceFactory;
class PacketResourceWatcher;
class PacketReferenceManager;
class PacketResourceStorage;

////////////////
// STRUCTURES //
////////////////

// Structures we know
struct PacketFile;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFileIndexer
////////////////////////////////////////////////////////////////////////////////
class PacketFileIndexer
{
protected:

    struct FileLoadInformation
    {
        Path             file_path;
        FileDataPosition file_data_position = 0;
        FileDataSize     file_data_size = 0;

        const PacketFileHeader&     file_header;
        const std::vector<uint8_t>& file_icon_data;
    };

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFileIndexer();
	~PacketFileIndexer();

//////////////////
// MAIN METHODS //
public: //////////

    // Initialize this indexer, populating its file map
    virtual bool Initialize(std::filesystem::path _resource_path) = 0;

    // Return if a given file is indexed by its path hash by this indexer
    virtual bool IsFileIndexed(HashPrimitive _file_hash) const = 0;

    // Return a file load information (its path, the location inside the file and its total size)
    virtual FileLoadInformation RetrieveFileLoadInformation(HashPrimitive _file_hash) const = 0;

    // Return a const reference to a file header
    virtual const PacketFileHeader& GetFileHeader(HashPrimitive _file_hash) const = 0;

    // Return a cost reference to a file icon data
    virtual const std::vector<uint8_t>& GetFileIconData(HashPrimitive _file_hash) const = 0;

protected:

///////////////
// VARIABLES //
private: //////


};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
