////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFile.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "../PacketConfig.h"
#include "PacketFileHeader.h"
#include "PacketFileReferences.h"

#include <string>
#include <unordered_map>
#include <assert.h>
#include <set>

///////////////
// NAMESPACE //
///////////////

// Packet data explorer
PacketDevelopmentNamespaceBegin(Packet)

// Classes we know
class PacketFileLoader;
class PacketPlainFileLoader;
class PacketFileSaver;
class PacketCompressedFileLoader;
class PacketReferenceManager;
class PacketFileManager;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFile
////////////////////////////////////////////////////////////////////////////////
class PacketFile
{
    // Friend classes
    friend PacketFileLoader;
    friend PacketFileSaver;
    friend PacketPlainFileLoader;
    friend PacketCompressedFileLoader;
    friend PacketFileManager;
    friend PacketReferenceManager;

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
    PacketFile();
	PacketFile(PacketFileHeader _fileHeaderReference, bool _is_internal_file);
	~PacketFile();

//////////////////
// MAIN METHODS //
public: //////////

    // Create a file from the given raw data
    static std::unique_ptr<PacketFile> CreateFileFromRawData(std::vector<uint8_t>&& _file_data, Path _file_path);

    // Transform a file into raw data
    static std::vector<uint8_t> CreateRawDataFromFile(std::unique_ptr<PacketFile> _file);

    // Convert an external file into a packet file, the resulting file will only have its final data set
    // and cannot be saved, to proper create a packet file the external one must be imported using
    // the PacketFileImporter class
    static std::unique_ptr<PacketFile> CreateFileFromExternal(std::vector<uint8_t>&& _file_data, Path _file_path);

    // Duplicate the given file, caution with this, change the path before saving it and if applicable, 
    // remove all links before doing it
    static std::unique_ptr<PacketFile> DuplicateFile(const std::unique_ptr<PacketFile>& _file);

    // Generate an internal file data from its separated data parts, all necessary links will be
    // added when attempting to save this file into disk
    static std::unique_ptr<PacketFile> GenerateFileFromData(
        Path                   _file_path,
        FileType               _file_type,
        std::vector<uint8_t>&& _icon_data,
        std::vector<uint8_t>&& _properties_data,
        std::vector<uint8_t>&& _original_data,
        std::vector<uint8_t>&& _intermediate_data,
        std::vector<uint8_t>&& _final_data,
        std::set<Path>&&       _file_dependencies);

    // Retrieve the final data for a file, this will consume the file object returning only its final
    // data as a vector, really useful when loading the file on condensed mode
    static std::vector<uint8_t> RetrieveFileFinalData(std::unique_ptr<PacketFile> _file);

    // Get this file icon data
    const std::vector<uint8_t>& GetIconData() const;

    // Return a const reference to the properties data
    const std::vector<uint8_t>& GetPropertiesData() const;

    // Return a const reference to the original data
    const std::vector<uint8_t>& GetOriginalData() const;

    // Return a const reference to the intermediate data
    const std::vector<uint8_t>& GetIntermediateData() const;

    // Return a const reference to the final data
    const std::vector<uint8_t>& GetFinalData() const;

    // Return a const reference to the references data
    const std::vector<uint8_t>& GetReferencesData() const;

    // Get this file header
    const PacketFileHeader& GetFileHeader() const;

    // Get this file properties json
    const nlohmann::json& GetFileProperties() const;

    // Get this file references
    const PacketFileReferences& GetFileReferences() const;

    // Return if this is an external file (not indexed)
    bool IsExternalFile() const;

protected:

    // Clear this file links, this should be used with extreme caution since it could
    // potentially result in invalid files, use only if copying a file and after its
    // new path was already set
    void ClearFileLinks();

    // Update a file data part
    bool UpdateFilePart(FilePart _file_part, std::vector<uint8_t>&& _data);

private:

    // Set this file data, must be called by a loader
    void SetData(std::vector<uint8_t>&& _iconData,
                 std::vector<uint8_t>&& _propertiesData,
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
    PacketFileHeader     m_FileHeader;
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
