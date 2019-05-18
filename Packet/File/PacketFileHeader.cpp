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
    file_header.m_OriginalPath = file_header_data.file_path;

    return file_header;
}

std::optional<PacketFileHeader> PacketFileHeader::CreateFromHeaderData(FileHeaderData _header_data)
{
    // Check if the magic number is right
    if (_header_data.magic != FileMagic)
    {
        return std::nullopt;
    }

    // Create the file header object and set its data
    PacketFileHeader file_header = {};
    file_header.m_HeaderData = _header_data;
    file_header.m_OriginalPath = _header_data.file_path;

    return file_header;
}

PacketFileHeader::FileHeaderData* PacketFileHeader::GetHeaderDataPtr(std::vector<uint8_t>& _data)
{
    // Check if the data can handle at least the header size
    if (_data.size() < sizeof(PacketFileHeader::FileHeaderData))
    {
        // Invalid file size!
        return nullptr;
    }

    auto* file_header_data = reinterpret_cast<PacketFileHeader::FileHeaderData*>(&_data[0]);

    // Check if the magic number is right
    if (file_header_data->magic != FileMagic)
    {
        return nullptr;
    }

    return file_header_data;
}

void PacketFileHeader::SetFileSize(FileDataSize _file_size)
{
    m_HeaderData.total_size = _file_size;
}

void PacketFileHeader::SetFileType(FileType _file_type)
{
    m_HeaderData.file_type = _file_type;
}

void PacketFileHeader::UpdateDataSizes(
    FileDataSize _icon_size,
    FileDataSize _properties_size,
    FileDataSize _original_size,
    FileDataSize _intermediate_size,
    FileDataSize _final_size,
    FileDataSize _references_size)
{
    m_HeaderData.icon_position              = sizeof(FileHeaderData);
    m_HeaderData.properties_position        = m_HeaderData.icon_position + _icon_size;
    m_HeaderData.original_data_position     = m_HeaderData.properties_position + _properties_size;
    m_HeaderData.intermediate_data_position = m_HeaderData.original_data_position + _original_size;
    m_HeaderData.final_data_position        = m_HeaderData.intermediate_data_position + _intermediate_size;
    m_HeaderData.references_data_position   = m_HeaderData.final_data_position + _final_size;
    m_HeaderData.total_size                 = m_HeaderData.references_data_position + _references_size;
}

std::vector<uint8_t> PacketFileHeader::GetRawData() const
{
    std::vector<uint8_t> header_data(sizeof(PacketFileHeader::FileHeaderData));
    *(reinterpret_cast<PacketFileHeader::FileHeaderData*>(header_data.data())) = m_HeaderData;
    return std::move(header_data);
}

void PacketFileHeader::SetPath(Path _file_path)
{
    m_HeaderData.file_path = _file_path;
    m_HeaderData.file_hash = Hash(_file_path);
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

Path PacketFileHeader::GetOriginalPath() const
{
    std::string original_path = m_OriginalPath.string();
    if (original_path.length() == 0)
    {
        return m_HeaderData.file_path;
    }

    return m_OriginalPath;
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