////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketPlainFileLoader.h"
#include "../Indexer/PacketFileIndexer.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketPlainFileLoader::PacketPlainFileLoader(const PacketFileIndexer& _file_indexer, std::filesystem::path _packet_path) :
    m_FileIndexer(_file_indexer), 
    PacketFileLoader(_packet_path)
{
}

PacketPlainFileLoader::~PacketPlainFileLoader()
{
}

std::unique_ptr<PacketFile> PacketPlainFileLoader::LoadFile(HashPrimitive _file_hash) const
{
    // Retrieve the file load information for this hash
    // TODO: This can be optimized since the LoadFileRawData() function will call the function below too
    auto file_load_info = m_FileIndexer.RetrieveFileLoadInformation(_file_hash);
    if (!file_load_info)
    {
        // Invalid hash/file not indexed
        return nullptr;
    }

    return PacketFile::CreateFileFromRawData(LoadFileRawData(_file_hash), file_load_info->file_path);
}

std::vector<uint8_t> PacketPlainFileLoader::LoadFileRawData(HashPrimitive _file_hash) const
{
    // Retrieve the file load information for this hash
    auto file_load_info = m_FileIndexer.RetrieveFileLoadInformation(_file_hash);
    if (!file_load_info)
    {
        // Invalid hash/file not indexed
        return {};
    }

    // Transform the file hash into a valid path
    std::filesystem::path file_path = MergeSystemPathWithFilePath(m_PacketPath, file_load_info->file_path);

    // Check if the file exist and is valid
    if (!std::filesystem::exists(file_path) || std::filesystem::is_directory(file_path))
    {
        // Invalid file
        return {};
    }

    // Open the file and check if we are ok to proceed
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open())
    {
        // Error opening the file!
        return {};
    }

    auto file_size = std::filesystem::file_size(file_path);

    std::vector<uint8_t> entire_file_data;
    entire_file_data.resize(file_size);

    if (!file.read(reinterpret_cast<char*>(entire_file_data.data()), file_size))
    {
        file.close();
        return {};
    }

    file.close();

    return entire_file_data;
}

std::optional<std::tuple<PacketFileHeader, std::vector<uint8_t>>> PacketPlainFileLoader::LoadFileDataPart(HashPrimitive _file_hash, FilePart _file_part) const
{
    // Retrieve the file load information for this hash
    auto file_load_info = m_FileIndexer.RetrieveFileLoadInformation(_file_hash);
    if (!file_load_info)
    {
        // Invalid hash/file not indexed
        return {};
    }

    // Transform the file hash into a valid path
    std::filesystem::path file_path = MergeSystemPathWithFilePath(m_PacketPath, file_load_info->file_path);

    // Check if the file exist and is valid
    if (!std::filesystem::exists(file_path) || std::filesystem::is_directory(file_path))
    {
        // Invalid file
        return std::nullopt;
    }

    // Open the file and check if we are ok to proceed
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open())
    {
        // Error opening the file!
        return std::nullopt;
    }

    // Load the file header data
    std::vector<uint8_t> file_header_data(sizeof(PacketFileHeader::FileHeaderData));
    if (!file.read(reinterpret_cast<char*>(&file_header_data[0]), file_header_data.size() * sizeof(uint8_t)))
    {
        return std::nullopt;
    }

    // Get the file header
    auto file_header = PacketFileHeader::CreateFromRawData(file_header_data);
    if (!file_header)
    {
        return std::nullopt;
    }

    // Determine the read location and the size for the selected file part
    FileDataPosition file_data_part_location = file_header->GetDataPosition(_file_part);
    FileDataSize file_data_part_size         = file_header->GetDataSize(_file_part);
    
    // See to the target location
    file.seekg(file_data_part_location, file.beg);

    // Read the target data
    std::vector<uint8_t> target_data(file_data_part_size);
    if (!file.read(reinterpret_cast<char*>(&target_data[0]), target_data.size() * sizeof(uint8_t)))
    {
        return std::nullopt;
    }

    {
        // Can't create directly
        std::tuple<PacketFileHeader, std::vector<uint8_t>> temp = { file_header.value(), std::move(target_data) };
        return std::move(temp);
    }
}