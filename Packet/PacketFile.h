////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFile.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
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

////////////////
// FORWARDING //
////////////////

// Classes we know
class PacketResourceFactory;
class PacketResourceWatcher;
class PacketReferenceManager;
class PacketResourceStorage;

////////////////
// STRUCTURES //
////////////////

struct FileHeader
{
    // Magic
    uint32_t magic = 699555;

    // Basic information
    uint32_t      version  = 0;
    FileType      fileType = 0;
    HashPrimitive fileHash = 0;
    // Last updated time
    // Other data

    // Data positions inside the file
    FileDataPosition iconPosition             = 0;
    FileDataPosition propertiesPosition       = 0;
    FileDataPosition originalDataPosition     = 0;
    FileDataPosition intermediateDataPosition = 0;
    FileDataPosition finalDataPosition        = 0;
    FileDataPosition referencesDataPosition   = 0;
};

struct FileReferences
{
    // Return a set of all files that depends on this one
    const std::set<std::string>& GetFilesThatDependsOnThis() const
    {
        return m_FilesThatDependsOnThis;
    }

    // Return a set of all files that this one depends on
    const std::set<std::string>& GetFileDependencies() const
    {
        return m_FileDependencies;
    }

    static FileReferences CreateFromData(const std::vector<uint8_t>& _data)
    {
        return CreateFromJSON(nlohmann::json::parse(_data.begin(), _data.end()));
    }

    static FileReferences CreateFromJSON(nlohmann::json _json)
    {
        FileReferences result;

        // Check if we have the necessary entries
        if (_json.count("FilesDependentOnThis") == 0 || _json.count("FileDependencies") == 0)
        {
            // Implicit move
            return result;
        }
        
        _json.at("FilesDependentOnThis").get_to(result.m_FilesThatDependsOnThis);
        _json.at("FileDependencies").get_to(result.m_FileDependencies);

        // Implicit move
        return result;
    }

protected:

    std::set<std::string> m_FilesThatDependsOnThis;
    std::set<std::string> m_FileDependencies;
};

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFile
////////////////////////////////////////////////////////////////////////////////
class PacketFile
{

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFile(const FileHeader& _fileHeaderReference, const std::vector<uint8_t>& _fileIconDataReference);
	~PacketFile();

//////////////////
// MAIN METHODS //
public: //////////

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
    const FileReferences& GetFileReferences() const;

protected:

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
private: //////

    // A reference to this file header and its icon that must be hold by the system
    const FileHeader&           m_FileHeaderReference;
    const std::vector<uint8_t>& m_FileIconDataReference;

    // Remaining file data
    std::vector<uint8_t> m_PropertiesData;
    std::vector<uint8_t> m_OriginalData;
    std::vector<uint8_t> m_IntermediateData;
    std::vector<uint8_t> m_FinalData;
    std::vector<uint8_t> m_ReferencesData;

    // The parsed properties data
    nlohmann::json m_ParsedPropertiesData;

    // The parsed references data
    FileReferences m_ParsedFileReferences;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
