////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileManager.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "../PacketConfig.h"
#include "Reference/PacketReferenceManager.h"
#include <string>
#include <map>
#include <set>
#include <mutex>
#include <shared_mutex>

// TODO: This should not be necessary after we ensure windows headers are not included
#undef CopyFile
#undef MoveFile
#undef DeleteFile

///////////////
// NAMESPACE //
///////////////

// Packet data explorer
PacketDevelopmentNamespaceBegin(Packet)

// Classes we know
class PacketFileIndexer;
class PacketFileLoader;
class PacketFileSaver;
class PacketFileConverter;
class PacketFileDefaultConverter;
class PacketFileImporter;
class PacketBackupManager;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFileManager
////////////////////////////////////////////////////////////////////////////////
class PacketFileManager
{
//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
    PacketFileManager(OperationMode _operation_mode,
                      BackupFlags _backup_flags,
                      std::filesystem::path _packet_path, 
                      std::filesystem::path _backup_path);
    ~PacketFileManager();

//////////////////
// MAIN METHODS //
public: //////////

    // Initialize this file importer
    bool Initialize();

    // Register a new converter
    void RegisterFileConverter(std::string _file_extension, std::unique_ptr<PacketFileConverter> _file_converter);

    // Return a reference to our objects
    PacketFileIndexer& GetFileIndexer()         const;
    const PacketFileLoader& GetFileLoader()     const;
    const PacketFileImporter& GetFileImporter() const;

/////////////////////////////
public: // FILE OPERATIONS //
/////////////////////////////

    // Write to a file, if the file doesn't exist it will be created if specified by the FileWriteFlags
    bool WriteFile(
        Path                   _target_path,
        std::vector<uint8_t>&& _icon_data,
        std::vector<uint8_t>&& _properties_data,
        std::vector<uint8_t>&& _original_data,
        std::vector<uint8_t>&& _intermediate_data,
        std::vector<uint8_t>&& _final_data,
        std::set<Path>&&       _file_dependencies,
        FileWriteFlags         _write_flags = FileWriteFlagBits::None) const;

    // Copy a file to another location, return the new file path
    std::optional<Path> CopyFile(Path _source_file_path, Path _target_file_dir) const;

    // Move a file to another location, return the file path
    std::optional<Path> MoveFile(Path _source_file_path, Path _target_file_dir) const;

    // Rename a file, the second argument must be only the filename without extension and not the
    // a path, return the renamed file path
    std::optional<Path> RenameFile(Path _source_file_path, Path _new_file_name) const;

    // Redirect all dependency links from one file to another, in other words this will
    // make any file that depends on the first one to depend on the second instead, it 
    // will change all references from the affected files
    bool RedirectFileDependencies(Path _source_file_path, Path _target_file_path);

    // Delete a file
    bool DeleteFile(Path _target_file_path) const;

///////////////////////
public: // CALLBACKS //
///////////////////////

    // Register the file operation failure callback
    void RegisterOperationFailureCallback(std::function<void(std::string, std::string, const PacketBackupManager& _backup_manager, std::set<Path>)> _callback);

    // Register the lock resource operations callback
    void RegisterLockResourceOperationsCallback(std::function<std::pair<std::unique_lock<std::shared_mutex>, std::unique_lock<std::mutex>>()> _callback);

///////////////////////////////
protected: // HELPER METHODS //
///////////////////////////////

    // Delete a file, optionally NOT removing its current dependency links, internal use only
    bool DeleteFile(Path _target_file_path, bool _remove_dependency_links) const;

    // Write a file data into an internal file, this doesn't check if the file should be overwritten
    bool WriteFileDataIntoInternalFile(Path _file_path, std::vector<uint8_t>&& _file_data) const;

    // Signal an operation error
    void SignalOperationError(std::string _operation, std::string _error_message) const;

private:

    // This method should be used when its necessary to extract path information about a file that is
    // being created (copied/moved/renamed), it will compare the source and target paths and output
    // the new target directory path, the filename and the file extension
    // This method will take in consideration if the target path points directly to a file path, in
    // this case the returned filename and extension will be the ones given by this path and not from
    // the source one
    std::tuple<std::filesystem::path, std::string, std::string> DetermineRequiredPaths(
        Path _source_file_path,
        Path _target_path) const;

    // Return a valid file path that can be used to save a file without overwriting an existing file
    Path RetrieveValidFilePath(const std::filesystem::path& _directory_path, const std::string& _file_name, const std::string& _file_extension) const;

///////////////
// VARIABLES //
private: //////

    // Our operation mode and backup flags
    OperationMode m_OperationMode;
    BackupFlags   m_BackupFlags;

    // Our packet path and our backup path
    std::filesystem::path m_PacketPath;
    std::filesystem::path m_BackupPath;

    // Our file management objects
    std::unique_ptr<PacketFileIndexer>      m_FileIndexer;
    std::unique_ptr<PacketFileLoader>       m_FileLoader;
    std::unique_ptr<PacketFileSaver>        m_FileSaver;
    std::unique_ptr<PacketFileImporter>     m_FileImporter;
    std::unique_ptr<PacketReferenceManager> m_FileReferenceManager;
    std::unique_ptr<PacketBackupManager>    m_BackupManager;

    // The default converter
    std::unique_ptr<PacketFileDefaultConverter> m_DefaultConverter;

    // Our map with all registered converters with their extensions
    std::map<std::string, std::unique_ptr<PacketFileConverter>> m_Converters;

    // Our callbacks
    std::function<void(std::string, std::string, const PacketBackupManager& _backup_manager, std::set<Path>)> m_OperationFailureCallback;
    std::function<std::pair<std::unique_lock<std::shared_mutex>, std::unique_lock<std::mutex>>()>             m_LockResourceOperationsCallback;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
