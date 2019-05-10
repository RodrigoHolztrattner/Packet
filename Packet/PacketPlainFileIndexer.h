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

// Classes we know
class PacketFileImporter;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketPlainFileIndexer
////////////////////////////////////////////////////////////////////////////////
class PacketPlainFileIndexer : public PacketFileIndexer
{
    // Friend classes
    friend PacketFileImporter;

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
    bool Initialize(std::filesystem::path _resource_path) final;

    // Return if a given file is indexed by its path hash by this indexer
    bool IsFileIndexed(HashPrimitive _file_hash) const final;

    // Return a file load information (its path, the location inside the file and its total size)
    FileLoadInformation RetrieveFileLoadInformation(HashPrimitive _file_hash) const final;

    // Return a const reference to a file header
    const FileHeader& GetFileHeader(HashPrimitive _file_hash) const final;

    // Return a cost reference to a file icon data
    const std::vector<uint8_t>& GetFileIconData(HashPrimitive _file_hash) const final;

protected:

    // Insert a resource index info into our map
    void InsertFileIndexData(Path _file_path);

    // Remove a resource index info from our map
    void RemoveFileIndexData(Path _file_path);

private:

    // Scan the resource path for files recursively and populate our info map
    void ScanSystemForFiles(std::filesystem::path _resource_path);

///////////////
// VARIABLES //
private: //////

    std::map<HashPrimitive, IndexData> m_IndexDatas;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
