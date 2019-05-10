////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketPlainFileLoader.h"
#include "PacketPlainFileIndexer.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketPlainFileLoader::PacketPlainFileLoader(const PacketFileIndexer& _file_indexer) :
    m_FileIndexer(_file_indexer)
{
}

PacketPlainFileLoader::~PacketPlainFileLoader()
{
}

std::unique_ptr<PacketFile> PacketPlainFileLoader::LoadFile(Hash _file_hash) const
{
    std::unique_ptr<PacketFile> result_file;

    // Transform the file hash into a valid path
    std::filesystem::path file_path = _file_hash.GetPath().String();

    // Check if the file exist and is valid
    if (!std::filesystem::exists(file_path) || std::filesystem::is_directory(file_path))
    {
        // Invalid file
        return nullptr;
    }

    // Open the file and check if we are ok to proceed
    std::ifstream file(_file_hash.GetPath().String(), std::ios::binary);
    if (!file.is_open())
    {
        // Error opening the file!
        return nullptr;
    }

    // Reserve space for the entire file
    std::vector<uint8_t> entire_file_data(std::filesystem::file_size(file_path));

    // Read the entire file data
    entire_file_data.insert(entire_file_data.begin(),
               std::istream_iterator<uint8_t>(file),
               std::istream_iterator<uint8_t>());

    // Close the file
    file.close();

    // Check if this is a packet file or any ordinary file
    // ...

    // Packet file
    {
        // Create the file from the loaded data
        result_file = PacketFile::CreateInternalFromData(std::move(entire_file_data));
    }
    // CreateExternalFromData

    return result_file;
}

std::vector<uint8_t> PacketPlainFileLoader::LoadFileRawData(Hash _file_hash) const
{
    // Transform the file hash into a valid path
    std::filesystem::path file_path = _file_hash.GetPath().String();

    // Check if the file exist and is valid
    if (!std::filesystem::exists(file_path) || std::filesystem::is_directory(file_path))
    {
        // Invalid file
        return {};
    }

    // Open the file and check if we are ok to proceed
    std::ifstream file(_file_hash.GetPath().String(), std::ios::binary);
    if (!file.is_open())
    {
        // Error opening the file!
        return {};
    }

    // Reserve space for the entire file
    std::vector<uint8_t> entire_file_data(std::filesystem::file_size(file_path));

    // Read the entire file data
    entire_file_data.insert(entire_file_data.begin(),
                            std::istream_iterator<uint8_t>(file),
                            std::istream_iterator<uint8_t>());

    // Close the file
    file.close();

    return entire_file_data;
}

std::optional<PacketFileReferences> PacketPlainFileLoader::LoadFileReferences(Hash _file_hash) const
{
    auto loaded_file = LoadFile(_file_hash);
    if (!loaded_file)
    {
        return std::nullopt;
    }

    return loaded_file->GetFileReferences();
}

/*

    Path file_path;
    FileDataPosition file_data_position = 0;
    FileDataSize file_data_size = 0;

        // Retrieve the file load information
        auto file_load_info = m_FileIndexer.RetrieveFileLoadInformation(_file_hash);

        // Set the file path, data position and data size
        file_path = file_load_info._file_path;
        file_data_position = file_load_info._file_data_position;
        file_data_size = file_load_info._file_data_size;

        // Initialize the result file
        result_file = std::make_unique<PacketFile>(file_load_info.file_header, file_load_info._file_icon_data);

*/