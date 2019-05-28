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
#include <mutex>
#include <shared_mutex>

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
class FileWatcherManager;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketPlainFileIndexer
////////////////////////////////////////////////////////////////////////////////
class PacketPlainFileIndexer : public PacketFileIndexer
{
    // Friend classes
    friend PacketFileManager;

    struct CacheData
    {
        // If this file is an external file
        bool file_is_external;

        // The icon data
        std::vector<uint8_t> icon_data;

        // The file properties json
        nlohmann::json file_properties;
    };

    struct IndexData
    {
        // The file extension
        std::string file_extension;

        // The file load information
        FileLoadInformation file_load_information;

        // The file optional cache (not always available)
        std::optional<CacheData> file_cache;
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

    // Return information about a given file
    std::optional<FileLoadInformation> RetrieveFileLoadInformation(HashPrimitive _file_hash) const final;
    std::optional<std::string> GetFileExtension(HashPrimitive _file_hash) const final;
    std::vector<uint8_t> GetFileIconData(HashPrimitive _file_hash) const final;
    nlohmann::json GetFileProperties(HashPrimitive _file_hash) const final;

    // Check some file property
    bool IsFileExternal(HashPrimitive _file_hash) const final;
    bool IsFileIndexed(HashPrimitive _file_hash) const final;

    // Query multiple files
    std::vector<Path> QueryFilesFromType(std::vector<std::string> _file_types) const final;
    std::vector<std::string> QueryRegisteredFileExtensions() const final;
    std::set<Path> QueryAllIndexedFiles() const final;

protected:

    // Register or update a file cache for the given file path, this will also index the file path is not
    // already indexed
    void RegisterFileCacheData(Path _file_path, nlohmann::json _file_properties, std::vector<uint8_t> _icon_data, bool _file_is_external);

    // Remove a resource index info from our map
    void RemoveFileIndexData(Path _file_path);

private:

    // Load and cache a file data from the given file path
    void LoadAndCacheFile(Path _file_path) const;

    // Index a file from the given file path
    void IndexFileFromPath(Path _file_path);

    // Index all files inside our packet path
    void BuildFilesystemView(std::filesystem::path _resource_path);

    // Return the cache data for the given file hash
    std::optional<CacheData> GetCacheForFileHash(HashPrimitive _file_hash) const;

///////////////
// VARIABLES //
private: //////

    // Our index data (mutable to allow caching while requesting data)
    mutable std::map<HashPrimitive, IndexData> m_IndexDatas;

    // All indexed files by their types
    std::map<std::string, std::set<Path>> m_IndexedFilesByType;

    // The file watcher manager, used to detect changes when an external
    // resource is modified
    std::unique_ptr<FileWatcherManager> m_FileWatcherManager;

    // The mutex used to synchronize all operations here
    mutable std::shared_mutex m_Mutex;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
