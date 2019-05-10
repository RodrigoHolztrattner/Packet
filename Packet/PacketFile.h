////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFile.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketFileReferences.h"
#include "Resource/PacketResourceManager.h"
#include <ctti//type_id.hpp>
#include <ctti//static_value.hpp>

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
class PacketFileLoader;
class PacketPlainFileLoader;
class PacketCompressedFileLoader;
class PacketReferenceManager;
class PacketFile;

////////////////
// STRUCTURES //
////////////////

struct FileHeader
{
    // Magic
    uint32_t magic = FileMagic;

    // Basic information
    uint32_t      version    = 0;
    FileType      file_type  = 0;
    HashPrimitive file_hash  = 0;
    FileDataSize  total_size = 0;
    // Last updated time
    // Other data

    // Data positions inside the file
    FileDataPosition icon_position              = 0;
    FileDataPosition properties_position        = 0;
    FileDataPosition original_data_position     = 0;
    FileDataPosition intermediate_data_position = 0;
    FileDataPosition final_data_position        = 0;
    FileDataPosition references_data_position   = 0;
};

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFile
////////////////////////////////////////////////////////////////////////////////
class PacketFile
{
    // Friend classes
    friend PacketFileLoader;
    friend PacketPlainFileLoader;
    friend PacketCompressedFileLoader;
    friend PacketReferenceManager;

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFile(FileHeader _fileHeaderReference, const std::vector<uint8_t> _fileIconDataReference, bool _is_internal_file);
	~PacketFile();

//////////////////
// MAIN METHODS //
public: //////////

    // Create an internal/external file from the given data
    static std::unique_ptr<PacketFile> CreateInternalFromData(std::vector<uint8_t>&& _file_data);
    static std::unique_ptr<PacketFile> CreateExternalFromData(std::vector<uint8_t>&& _file_data);

    // Generate an internal file data from its separated data parts
    static std::vector<uint8_t> GenerateInternalFileData(
        HashPrimitive          _file_hash,
        FileType               _file_type,
        std::vector<uint8_t>&& _icon_data,
        std::vector<uint8_t>&& _properties_data,
        std::vector<uint8_t>&& _original_data,
        std::vector<uint8_t>&& _intermediate_data,
        std::vector<uint8_t>&& _final_data,
        std::set<Path>&&       _files_that_depends_on_this,
        std::set<Path>&&       _file_dependencies);

    // Break a file into its small data, this will consume the file unique ptr
    static bool BreakFileIntoDatas(
        std::unique_ptr<PacketFile> _file,
        std::vector<uint8_t>&       _icon_data,
        std::vector<uint8_t>&       _properties_data,
        std::vector<uint8_t>&       _original_data,
        std::vector<uint8_t>&       _intermediate_data,
        std::vector<uint8_t>&       _final_data,
        PacketFileReferences&       _file_references_parsed);

    // Transform a file into raw data
    static std::vector<uint8_t> TransformFileIntoRawData(std::unique_ptr<PacketFile> _file);

    // Return a const reference to the original data
    const std::vector<uint8_t>& GetOriginalData() const;

    // Return a const reference to the intermediate data
    const std::vector<uint8_t>& GetIntermediateData() const;

    // Return a const reference to the final data
    const std::vector<uint8_t>& GetFinalData() const;

    // Get this file icon data
    const std::vector<uint8_t>& GetIconData() const;

    // Get this file header
    const FileHeader& GetFileHeader() const;

    // Get this file properties json
    const nlohmann::json& GetFileProperties() const;

    // Get this file references
    const PacketFileReferences& GetFileReferences() const;

    // Return if this is an internal file (not indexed)
    bool IsInternalFile() const;

protected:

    // Get this file references (non-const)
    PacketFileReferences& GetNonConstFileReferences();

    // Set this file data, must be called by a loader
    void SetData(std::vector<uint8_t>&& _propertiesData, 
                 std::vector<uint8_t>&& _originalData,
                 std::vector<uint8_t>&& _intermediateData,
                 std::vector<uint8_t>&& _finalData, 
                 std::vector<uint8_t>&& _referencesData);

    // Parse the properties data into a json format
    void ParseProperties();

    // Parse the file references
    void ParseReferences();

///////////////
// VARIABLES //
protected: ////

    // A reference to this file header and its icon that must be hold by the system
    FileHeader           m_FileHeader;
    std::vector<uint8_t> m_FileIconData;

    // Remaining file data
    std::vector<uint8_t> m_PropertiesData;
    std::vector<uint8_t> m_OriginalData;
    std::vector<uint8_t> m_IntermediateData;
    std::vector<uint8_t> m_FinalData;
    std::vector<uint8_t> m_ReferencesData;

    // The parsed properties data
    nlohmann::json m_ParsedPropertiesData;

    // The parsed references data
    PacketFileReferences m_ParsedFileReferences;

    // If this is an internal file (not indexed)
    bool m_IsInternalFile = false;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
