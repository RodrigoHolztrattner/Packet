////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileSaver.h"
#include "PacketPlainFileIndexer.h"
#include "File/PacketFile.h"
#include "File/PacketFileHeader.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketFileSaver::PacketFileSaver(const PacketFileIndexer& _file_indexer, std::filesystem::path _packet_path) :
    m_PacketPath(_packet_path), 
    m_FileIndexer(_file_indexer)
{
}

PacketFileSaver::~PacketFileSaver()
{
}

bool PacketFileSaver::SaveFile(std::unique_ptr<PacketFile> _file) const
{
    // Check if the file is valid
    if (!_file)
    {
        return false;
    }

    // Determine the system path this file must be saved on
    auto file_output_path = MergeSystemPathWithFilePath(m_PacketPath, _file->GetFileHeader().GetPath());

    // Transform the file into raw data
    auto file_raw_data = PacketFile::TransformIntoRawData(std::move(_file));

    // Open the file and check if we are ok to proceed
    std::ofstream system_file(file_output_path, std::ios::binary);
    if (!system_file.is_open())
    {
        // Error opening the file!
        return false;
    }

    // Write the file data
    system_file.write(reinterpret_cast<char*>(file_raw_data.data()), file_raw_data.size() * sizeof(uint8_t));

    // Close the file, operation success
    system_file.close();

    return true;
}

bool PacketFileSaver::SaveFile(std::vector<uint8_t>&& _file_raw_data) const
{
    // Check if the file data is valid
    if (_file_raw_data.size() == 0)
    {
        return false;
    }

    // Get the file header from the data
    auto file_header = PacketFileHeader::CreateFromRawData(_file_raw_data);
    if (!file_header)
    {
        return false;
    }

    // Determine the system path this file must be saved on
    auto file_output_path = MergeSystemPathWithFilePath(m_PacketPath, file_header->GetPath());

    // Open the file and check if we are ok to proceed
    std::ofstream system_file(file_output_path, std::ios::binary);
    if (!system_file.is_open())
    {
        // Error opening the file!
        return false;
    }

    // Write the file data
    system_file.write(reinterpret_cast<char*>(_file_raw_data.data()), _file_raw_data.size() * sizeof(uint8_t));

    // Close the file, operation success
    system_file.close();

    return true;
}

bool PacketFileSaver::SaveFile(const PacketFileHeader& _file_header, FilePart _file_part, std::vector<uint8_t>&& _file_data_part) const
{
    return false;
}