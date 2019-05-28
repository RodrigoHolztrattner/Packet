////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileImporter.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "../../PacketConfig.h"
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
class PacketFileConverter;
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
    PacketFileImporter(
        PacketFileIndexer&                                _file_indexer,
        const PacketFileLoader&                           _file_loader,
        const PacketReferenceManager&                     _reference_manager,
        FileWriteCallback                                 _file_write_callback,
        std::function<PacketFileConverter* (std::string)> _retrieve_converter_for_type_callback,
        std::filesystem::path                             _packet_path);
	~PacketFileImporter();

//////////////////
// MAIN METHODS //
public: //////////

    // Initialize this file importer
    bool Initialize();

    // Import an external file
    std::optional<Path> ImportExternalFile(std::filesystem::path _file_original_path, Path _target_dir, FileImportFlags _import_flags = 0) const;

protected:

///////////////
// VARIABLES //
private: //////

    // A reference to our file indexer, file loader and reference manager
    PacketFileIndexer& m_FileIndexer;
    const PacketFileLoader& m_FileLoader;
    const PacketReferenceManager& m_ReferenceManager;

    // Our packet path
    std::filesystem::path m_PacketPath;

    // The default converter
    std::unique_ptr<PacketFileConverter> m_DefaultConverter;

    // Our map with all registered converters with their extensions
    std::map<std::string, std::unique_ptr<PacketFileConverter>> m_Converters;

    // Our callbacks
    FileWriteCallback                                 m_FileWriteCallback;
    std::function<PacketFileConverter* (std::string)> m_RetrieveConverterForTypeCallback;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
