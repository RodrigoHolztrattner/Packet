////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileIndexer.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "../../PacketConfig.h"
#include "../PacketFileHeader.h"
#include "../Reference/PacketFileReferences.h"
#include "../PacketFile.h"
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

////////////////
// STRUCTURES //
////////////////

// Structures we know
struct PacketFile;

// Classes we know
class PacketFileLoader;

// The file modification callback type
typedef std::function<void(const Path&)> FileModificationCallback;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFileIndexer
////////////////////////////////////////////////////////////////////////////////
class PacketFileIndexer
{
protected:

    struct FileLoadInformation
    {
        Path             file_path;
        HashPrimitive    file_hash;
        FileDataPosition file_data_position = 0;
        FileDataSize     file_data_size     = 0;
    };

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFileIndexer(std::filesystem::path _packet_path);
	virtual ~PacketFileIndexer();

//////////////////
// MAIN METHODS //
public: //////////

    // Set auxiliary object pointers
    void SetAuxiliarObjects(const PacketFileLoader* _file_loader);

//////////////////////////
public: // FILE WATCHER //
//////////////////////////

    // Register a file modification callback
    void RegisterFileModificationCallback(FileModificationCallback _callback);

    // Register a file watcher
    template <class WatcherClass, typename ... Args>
    PacketFileWatcher* RegisterFileWatcher(HashPrimitive _file_hash, Args&& ... args)
    {
        static_assert(std::is_base_of<PacketFileWatcher, WatcherClass>::value);
        std::lock_guard lock(m_Mutex);

        // Create a new watcher class and insert it into our watcher map
        auto file_watcher = std::make_unique<WatcherClass>(_file_hash, std::forward<Args>(args) ...);
        auto file_watcher_ptr = file_watcher.get();
        m_FileWatchers[_file_hash].insert({ file_watcher_ptr, std::move(file_watcher) });

        return file_watcher_ptr;
    }

    // Unregister a file watcher
    void UnregisterFileWatcher(PacketFileWatcher* _file_watcher_ptr);

protected:

    // Propagate a file modification to anyone who could be watching it, this method
    // doesn't lock the main mutex, this synchronization should be guaranteed by the
    // caller
    void PropagateFileModificationToWatchers(Path _file_path) const;

/////////////////////////////
public: // VIRTUAL METHODS //
/////////////////////////////

    // Initialize this indexer, populating its file map
    virtual bool Initialize() = 0;

    // Return information about a given file
    virtual std::optional<FileLoadInformation> RetrieveFileLoadInformation(HashPrimitive _file_hash) const = 0;
    virtual std::optional<std::string> GetFileExtension(HashPrimitive _file_hash)                    const = 0;
    virtual std::vector<uint8_t> GetFileIconData(HashPrimitive _file_hash)                           const = 0;
    virtual nlohmann::json GetFileProperties(HashPrimitive _file_hash)                               const = 0;

    // Check some file property
    virtual bool IsFileExternal(HashPrimitive _file_hash) const = 0;
    virtual bool IsFileIndexed(HashPrimitive _file_hash)  const = 0;

    // Return a valid path for the given path, name and extension
    virtual Path GetValidPathForName(const Path& _current_path, std::string _name, std::string _extension) const = 0;

    // Query multiple files
    virtual std::vector<Path> QueryFilesFromType(std::vector<std::string> _file_types) const = 0;
    virtual std::vector<std::string> QueryRegisteredFileExtensions()                   const = 0;
    virtual std::set<Path> QueryAllIndexedFiles()                                      const = 0;

///////////////
// VARIABLES //
protected: ////

    // The packet path
    std::filesystem::path m_PacketPath;

    // The file loader references
    const PacketFileLoader* m_FileLoaderPtr = nullptr;

    // Our callbacks (only active on Plain move)
    std::vector<FileModificationCallback> m_FileModificationCallbacks;

    // Our file watchers
    std::map<HashPrimitive, std::map<PacketFileWatcher*, std::unique_ptr<PacketFileWatcher>>> m_FileWatchers;

    // The mutex used to synchronize all operations here
    mutable std::shared_mutex m_Mutex;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
