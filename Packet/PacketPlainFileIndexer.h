////////////////////////////////////////////////////////////////////////////////
// Filename: PacketPlainFileIndexer.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketFileIndexer.h"
#include "PacketFile.h"

#include <string>
#include <unordered_map>
#include <assert.h>
#include <set>

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
// Class name: PacketPlainFileIndexer
////////////////////////////////////////////////////////////////////////////////
class PacketPlainFileIndexer : public PacketFileIndexer
{
    struct IndexData
    {
        // The file header
        FileHeader file_header;

        // The icon data
        std::vector<uint8_t> icon_data;

        // This file references
        PacketFileReferences file_references;
    };

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketPlainFileIndexer();
	~PacketPlainFileIndexer();

//////////////////
// MAIN METHODS //
public: //////////

    // Initialize this indexer, populating its file map
    virtual bool Initialize(std::filesystem::path _resource_path) = 0;

    // Return if a given file is indexed by its path hash by this indexer
    virtual bool IsFileIndexed(HashPrimitive _file_hash) const = 0;

    // Return a const reference to a file header
    virtual const FileHeader& GetFileHeader(HashPrimitive _file_hash) const = 0;

    // Return a cost reference to a file icon data
    virtual const std::vector<uint8_t>& GetFileIconData(HashPrimitive _file_hash) const = 0;

private:

    // Scan the resource path for files recursively and populate our info map
    void ScanResourcePath(std::filesystem::path _resource_path);

    // Insert a resource index info into our map
    void InsertResourceIndexData(std::filesystem::path _resource_path);

///////////////
// VARIABLES //
private: //////

    std::map<HashPrimitive, IndexData> m_IndexDatas;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
