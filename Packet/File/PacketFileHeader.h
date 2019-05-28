////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileHeader.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "../PacketConfig.h"

///////////////
// NAMESPACE //
///////////////

// Packet data explorer
PacketDevelopmentNamespaceBegin(Packet)

// Classes we know
class PacketFile;
class PacketFileSaver;
class PacketPlainFileLoader;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFileHeader
////////////////////////////////////////////////////////////////////////////////
class PacketFileHeader
{
    // Friend classes
    friend PacketFile;
    friend PacketFileSaver;
    friend PacketPlainFileLoader;

public:

    struct FileHeaderData
    {
        // Magic
        uint32_t magic = FileMagic;

        // Basic information
        uint32_t      version    = PacketVersion;
        Path          file_path;
        HashPrimitive file_hash  = 0;
        FileDataSize  total_size = sizeof(FileHeaderData);
        // Creation Time
        // Update Time
        // Checksum

        // Data positions inside the file
        FileDataPosition icon_position              = 0;
        FileDataPosition properties_position        = 0;
        FileDataPosition original_data_position     = 0;
        FileDataPosition intermediate_data_position = 0;
        FileDataPosition final_data_position        = 0;
        FileDataPosition references_data_position   = 0;
    };

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFileHeader();
	~PacketFileHeader();

//////////////////
// MAIN METHODS //
public: //////////

    // Set the file path, any file link will only be adjusted when saving this file,
    // the only collateral change this will make is also changing the hash
    void SetPath(Path _file_path);

    // Return this header info
    uint32_t GetVersion()      const;
    Path GetPath()             const;
    Path GetOriginalPath()     const;
    HashPrimitive GetHash()    const;
    FileDataSize GetFileSize() const;

    // Return the file data position/size for the given file part
    FileDataPosition GetDataPosition(FilePart _file_part) const;
    FileDataSize GetDataSize(FilePart _file_part)         const;

protected:

    // Create a header from the given file raw data, the data must be valid
    static std::optional<PacketFileHeader> CreateFromRawData(const std::vector<uint8_t>& _data);

    // Create a header from the given header data
    static std::optional<PacketFileHeader> CreateFromHeaderData(FileHeaderData _header_data);

    // Return a header data pointer to the given file raw data, if valid
    static FileHeaderData* GetHeaderDataPtr(std::vector<uint8_t>& _data);

    // Set this header info
    void SetFileSize(FileDataSize _file_size);
    void UpdateDataSizes(
        FileDataSize _icon_size,
        FileDataSize _properties_size,
        FileDataSize _original_size,
        FileDataSize _intermediate_size,
        FileDataSize _final_size,
        FileDataSize _references_size);

    // Get this header as pure data
    std::vector<uint8_t> GetRawData() const;

///////////////
// VARIABLES //
protected: ////

    // The header data
    FileHeaderData m_HeaderData;

    // The original path this header had when it was created, used mainly to update references
    // when updating a file
    Path m_OriginalPath;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
