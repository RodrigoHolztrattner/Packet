////////////////////////////////////////////////////////////////////////////////
// Filename: PacketPlainFileIndexer.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "../../PacketConfig.h"
#include "PacketFileIndexer.h"
#include "../PacketFileHeader.h"
#include "../Reference/PacketFileReferences.h"
#include "../PacketFile.h"

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
class PacketFileManager;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketPlainFileIndexer
////////////////////////////////////////////////////////////////////////////////
class PacketPlainFileIndexer : public PacketFileIndexer
{
    // Friend classes
    friend PacketFileManager;

    struct IndexData
    {
        // The file header
        std::optional<PacketFileHeader> file_header;

        // The icon data
        std::vector<uint8_t> icon_data;

        // The file extension
        std::string file_extension;

        // The file properties json
        nlohmann::json file_properties;

        // The file load information
        FileLoadInformation file_load_information;
    };

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketPlainFileIndexer(std::filesystem::path _packet_path);
	~PacketPlainFileIndexer();

//////////////////
// MAIN METHODS //
public: //////////

    // Initialize this indexer, populating its file map
    bool Initialize() final;

    // Return if a given file is indexed by its path hash by this indexer
    bool IsFileIndexed(HashPrimitive _file_hash) const final;

    // Return a file load information (its path, the location inside the file and its total size)
    std::optional<FileLoadInformation> RetrieveFileLoadInformation(HashPrimitive _file_hash) const final;

    // Return a file extension, if applicable
    std::optional<std::string> GetFileExtension(HashPrimitive _file_hash) const final;

    // Return a file header, if applicable
    std::optional<PacketFileHeader> GetFileHeader(HashPrimitive _file_hash) const final;

    // Return a file icon data
    std::vector<uint8_t> GetFileIconData(HashPrimitive _file_hash) const final;

    // Return a file properties
    nlohmann::json GetFileProperties(HashPrimitive _file_hash) const final;

protected:

    // Insert a resource index info into our map
    void InsertFileIndexData(Path _file_path);

    // Remove a resource index info from our map
    void RemoveFileIndexData(Path _file_path);

private:

    // Index all files inside our packet path
    void BuildFilesystemView(std::filesystem::path _resource_path);

///////////////
// VARIABLES //
private: //////

    // Our index data
    std::map<HashPrimitive, IndexData> m_IndexDatas;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
