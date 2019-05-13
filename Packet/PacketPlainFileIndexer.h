////////////////////////////////////////////////////////////////////////////////
// Filename: PacketPlainFileIndexer.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketFileIndexer.h"
#include "File/PacketFileHeader.h"
#include "File/PacketFileReferences.h"
#include "File/PacketFile.h"

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

    struct DirectoryNode
    {
        friend PacketPlainFileIndexer;

        DirectoryNode() = default;

        bool operator== (const DirectoryNode& _other)
        {
            return (directory_name == _other.directory_name &&
                    directory_internal_path == _other.directory_internal_path &&
                    children_folders == _other.children_folders);
        }

        Path                                        directory_name;
        Path                                        directory_path;
        std::vector<std::unique_ptr<DirectoryNode>> children_folders;
        std::vector<Path>                           children_files;

    protected:

        std::filesystem::path directory_internal_path;

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

    // Scan the resource path for files and folders recursively and populate our info map
    void BuildFilesystemView(std::filesystem::path _resource_path);

///////////////
// VARIABLES //
private: //////

    // Our index data
    std::map<HashPrimitive, IndexData> m_IndexDatas;

    // The directory root node
    std::unique_ptr<DirectoryNode> m_RootNode;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
