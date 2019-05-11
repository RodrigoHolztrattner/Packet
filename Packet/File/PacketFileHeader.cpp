////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileHeader.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileHeader.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketFileHeader::PacketFileHeader()
{
	// Set the initial data
	// ...
}

PacketFileHeader::~PacketFileHeader()
{
}

std::optional<PacketFileHeader> PacketFileHeader::CreateFromRawData(const std::vector<uint8_t>& _data)
{
    // Check if the data can handle at least the header size
    if (_data.size() < sizeof(PacketFileHeader::FileHeaderData))
    {
        // Invalid file size!
        return std::nullopt;
    }

    // Read the file header
    auto& file_header_data = *reinterpret_cast<const PacketFileHeader::FileHeaderData*>(&_data[0]);

    // Check if the magic number is right
    if (file_header_data.magic != FileMagic)
    {
        return std::nullopt;
    }

    // Create the file header object and set its data
    PacketFileHeader file_header = {};
    file_header.m_HeaderData = file_header_data;

    return file_header;
}

std::vector<uint8_t> PacketFileHeader::GetRawData() const
{
    std::vector<uint8_t> header_data(sizeof(PacketFileHeader::FileHeaderData));
    *(reinterpret_cast<PacketFileHeader::FileHeaderData*>(header_data.data())) = m_HeaderData;
    return std::move(header_data);
}

uint32_t PacketFileHeader::GetVersion() const
{
    return m_HeaderData.version;
}

FileType PacketFileHeader::GetFileType() const
{
    return m_HeaderData.file_type;
}

Path PacketFileHeader::GetPath() const
{
    return m_HeaderData.file_path;
}

HashPrimitive PacketFileHeader::GetHash() const
{
    return m_HeaderData.file_hash;
}

FileDataSize PacketFileHeader::GetFileSize() const
{
    return m_HeaderData.total_size;
}

FileDataPosition PacketFileHeader::GetDataPosition(FilePart _file_part) const
{
    switch (_file_part)
    {
        case FilePart::Header:
        {
            return 0;
        }
        case FilePart::IconData:
        {
            return m_HeaderData.icon_position;
        }
        case FilePart::PropertiesData:
        {
            return m_HeaderData.properties_position;
        }
        case FilePart::OriginalData:
        {
            return m_HeaderData.original_data_position;
        }
        case FilePart::IntermediateData:
        {
            return m_HeaderData.intermediate_data_position;
        }
        case FilePart::FinalData:
        {
            return m_HeaderData.final_data_position;
        }
        case FilePart::ReferencesData:
        {
            return m_HeaderData.references_data_position;
        }
    }

    throw "Invalid file part";
    return -1; // Is this necessary?
}

FileDataSize PacketFileHeader::GetDataSize(FilePart _file_part) const
{
    switch (_file_part)
    {
        case FilePart::Header:
        {
            return sizeof(PacketFileHeader::FileHeaderData);
        }
        case FilePart::IconData:
        {
            return m_HeaderData.properties_position - m_HeaderData.icon_position;
        }
        case FilePart::PropertiesData:
        {
            return m_HeaderData.original_data_position - m_HeaderData.properties_position;
        }
        case FilePart::OriginalData:
        {
            return m_HeaderData.intermediate_data_position - m_HeaderData.original_data_position;
        }
        case FilePart::IntermediateData:
        {
            return m_HeaderData.final_data_position - m_HeaderData.intermediate_data_position;
        }
        case FilePart::FinalData:
        {
            return m_HeaderData.references_data_position - m_HeaderData.final_data_position;
        }
        case FilePart::ReferencesData:
        {
            return m_HeaderData.total_size - m_HeaderData.references_data_position;
        }
    }

    throw "Invalid file part";
    return -1; // Is this necessary?
}