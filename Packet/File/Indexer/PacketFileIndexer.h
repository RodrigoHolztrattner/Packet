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
        FileDataSize     file_data_size = 0;
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

    // Register a file modification callback
    void RegisterFileModificationCallback(FileModificationCallback _callback);

/////////////////////////////
public: // VIRTUAL METHODS //
/////////////////////////////

    // Initialize this indexer, populating its file map
    virtual bool Initialize() = 0;

    // Return if a given file is indexed by its path hash by this indexer
    virtual bool IsFileIndexed(HashPrimitive _file_hash) const = 0;

    // Return a file load information (its path, the location inside the file and its total size)
    virtual std::optional<FileLoadInformation> RetrieveFileLoadInformation(HashPrimitive _file_hash) const = 0;

    // Return a file extension, if applicable
    virtual std::optional<std::string> GetFileExtension(HashPrimitive _file_hash) const = 0;

    // Return a file icon data
    virtual std::vector<uint8_t> GetFileIconData(HashPrimitive _file_hash) const = 0;

    // Return a file properties
    virtual nlohmann::json GetFileProperties(HashPrimitive _file_hash) const = 0;

    // Return if a file is an external file
    virtual bool IsFileExternal(HashPrimitive _file_hash) const = 0;

    // Return the paths for all indexed files
    virtual std::set<Path> GetAllIndexedFiles() const = 0;

///////////////
// VARIABLES //
protected: ////

    // The packet path
    std::filesystem::path m_PacketPath;

    // The file loader references
    const PacketFileLoader* m_FileLoaderPtr = nullptr;

    // Our callbacks (only active on Plain move)
    std::vector<FileModificationCallback>                         m_FileModificationCallbacks;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
