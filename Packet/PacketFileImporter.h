////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileImporter.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketFileConverter.h"
#include <string>
#include <map>
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

// Import file flags
enum class FileImportFlagBits
{
    None,
    Overwrite
};

// Write file flags
enum class FileWriteFlagBits
{
    None, 
    CreateIfInexistent
};

typedef uint32_t FileImportFlags;
typedef uint32_t FileWriteFlags;

// Classes we know
class PacketFileIndexer;
class PacketFileLoader;
class PacketReferenceManager;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFileImporter
////////////////////////////////////////////////////////////////////////////////
class PacketFileImporter
{

    // Friend classes
    friend PacketReferenceManager;

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFileImporter(PacketFileIndexer& _file_indexer, 
                       const PacketFileLoader& _file_loader,
                       const PacketReferenceManager& _reference_manager, 
                       std::wstring _resource_path);
	~PacketFileImporter();

//////////////////
// MAIN METHODS //
public: //////////

    // Initialize this file importer
    bool Initialize();

    // Register a new converter
    void RegisterFileConverter(std::string _file_extension, std::unique_ptr<PacketFileConverter> _file_converter);

    // Import an external file
    bool ImportExternalFile(std::filesystem::path _file_original_path, Path _target_path, FileImportFlags _import_flags = 0) const;

    // Write to an internal file, if the file doesn't exist it will be created if specified by the FileWriteFlags
    bool WriteInternalFile(
        Path                   _target_path,
        FileType               _file_type,
        std::vector<uint8_t>&& _icon_data,
        std::vector<uint8_t>&& _properties_data,
        std::vector<uint8_t>&& _original_data,
        std::vector<uint8_t>&& _intermediate_data,
        std::vector<uint8_t>&& _final_data,
        std::set<Path>&&       _file_dependencies,
        FileWriteFlags         _write_flags = 0) const;

    // Copy an internal file to another location
    bool CopyInternalFile(Path _source_file_path, Path _target_file_path) const;

    // Move an internal file to another location
    bool MoveInternalFile(Path _source_file_path, Path _target_file_path) const;

protected:

    // Write a file data into an internal file, this doesn't check if the file should be overwritten
    bool WriteFileDataIntoInternalFile(Path _file_path, std::vector<uint8_t>&& _file_data) const;

///////////////
// VARIABLES //
private: //////

    // A reference to our file indexer, file loader and reference manager
    PacketFileIndexer& m_FileIndexer;
    const PacketFileLoader& m_FileLoader;
    const PacketReferenceManager& m_ReferenceManager;

    // Our resource path
    std::wstring m_ResourcePath;

    // The default converter
    std::unique_ptr<PacketFileConverter> m_DefaultConverter;

    // Our map with all registered converters with their extensions
    std::map<std::string, std::unique_ptr<PacketFileConverter>> m_Converters;

};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
