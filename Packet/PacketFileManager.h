////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileManager.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
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

// Classes we know
class PacketFileIndexer;
class PacketFileLoader;
class PacketFileSaver;
class PacketFileConverter;
class PacketFileImporter;
class PacketReferenceManager;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFileManager
////////////////////////////////////////////////////////////////////////////////
class PacketFileManager
{
//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFileManager(OperationMode _operation_mode, std::wstring _resource_path);
	~PacketFileManager();

//////////////////
// MAIN METHODS //
public: //////////

    // Initialize this file importer
    bool Initialize();

    // Register a new converter
    void RegisterFileConverter(std::string _file_extension, std::unique_ptr<PacketFileConverter> _file_converter);

    // Write to a file, if the file doesn't exist it will be created if specified by the FileWriteFlags
    bool WriteFile(
        Path                   _target_path,
        FileType               _file_type,
        std::vector<uint8_t>&& _icon_data,
        std::vector<uint8_t>&& _properties_data,
        std::vector<uint8_t>&& _original_data,
        std::vector<uint8_t>&& _intermediate_data,
        std::vector<uint8_t>&& _final_data,
        std::set<Path>&&       _file_dependencies,
        FileWriteFlags         _write_flags = 0) const;

    // Copy a file to another location
    bool CopyFile(Path _source_file_path, Path _target_file_path) const;

    // Move a file to another location
    bool MoveFile(Path _source_file_path, Path _target_file_path) const;

    // Delete a file
    bool DeleteFile(Path _target_file_path) const;

protected:

    // Write a file data into an internal file, this doesn't check if the file should be overwritten
    bool WriteFileDataIntoInternalFile(Path _file_path, std::vector<uint8_t>&& _file_data) const;

///////////////
// VARIABLES //
private: //////

    // Our operation mode
    OperationMode m_OperationMode;

    // Our packet path
    std::wstring m_PacketPath;

    // Our file management objects
    std::unique_ptr<PacketFileIndexer>      m_FileIndexer;
    std::unique_ptr<PacketFileLoader>       m_FileLoader;
    std::unique_ptr<PacketFileSaver>        m_FileSaver;
    std::unique_ptr<PacketFileImporter>     m_FileImporter;
    std::unique_ptr<PacketReferenceManager> m_FileReferenceManager;

    // The default converter
    std::unique_ptr<PacketFileConverter> m_DefaultConverter;

    // Our map with all registered converters with their extensions
    std::map<std::string, std::unique_ptr<PacketFileConverter>> m_Converters;

};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
