////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileImporter.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileImporter.h"
#include "../Indexer/PacketPlainFileIndexer.h"
#include "../Converter/PacketFileConverter.h"
#include "../PacketFile.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketFileImporter::PacketFileImporter(
    PacketFileIndexer&                                _file_indexer,
    const PacketFileLoader&                           _file_loader,
    const PacketReferenceManager&                     _reference_manager,
    FileWriteCallback                                 _file_write_callback,
    std::function<PacketFileConverter* (std::string)> _retrieve_converter_for_type_callback,
    std::filesystem::path                             _packet_path) :
    m_FileIndexer(_file_indexer),
    m_FileLoader(_file_loader), 
    m_ReferenceManager(_reference_manager), 
    m_FileWriteCallback(_file_write_callback), 
    m_RetrieveConverterForTypeCallback(_retrieve_converter_for_type_callback),
    m_PacketPath(_packet_path)
{
	// Set the initial data
	// ...
}

PacketFileImporter::~PacketFileImporter()
{
}

bool PacketFileImporter::Initialize()
{

    return true;
}

std::optional<Path> PacketFileImporter::ImportExternalFile(std::filesystem::path _file_original_path, Path _target_dir, FileImportFlags _import_flags) const
{
    // Check if the file exist and is valid
    if (!std::filesystem::exists(_file_original_path) || std::filesystem::is_directory(_file_original_path))
    {
        // Invalid file
        return std::nullopt;
    }

    // Setup the target path and verify if it's valid and a directory
    auto target_system_path = MergeSystemPathWithFilePath(m_PacketPath, _target_dir);
    if (!std::filesystem::exists(target_system_path) || !std::filesystem::is_directory(target_system_path))
    {
        // Invalid target path
        return std::nullopt;
    }

    // Check if we have a converter for this file extension
    PacketFileConverter* converter = m_RetrieveConverterForTypeCallback(_file_original_path.extension().string());
    if (!converter)
    {
        return std::nullopt;
    }

    // Setup the target complete path
    Path taget_path = CreateLocalPathFromExternal(_file_original_path, _target_dir, converter->GetConversionFileExtension().string());

    // Check if we already have this file imported, if true, check if we must overwrite it
    bool file_already_indexed = m_FileIndexer.IsFileIndexed(Hash(taget_path));
    if (file_already_indexed && !(_import_flags & static_cast<FileImportFlags>(FileImportFlagBits::Overwrite)))
    {
        // The file already exist and we are not overwriting it
        return std::nullopt;
    }
    else if (file_already_indexed)
    {
        // Replace the old file
        // ...
        throw "Not implemented yet";
    }

    // Open the file and check if we are ok to proceed
    std::ifstream file(_file_original_path, std::ios::binary);
    if (!file.is_open())
    {
        // Error opening the file!
        return std::nullopt;
    }

    // Read the entire file data
    std::vector<uint8_t> entire_file_data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Close the file
    file.close();

    // Convert the external file and retrieve its internal data
    std::vector<uint8_t> entire_file_data_copy = entire_file_data;
    std::vector<uint8_t> icon_data;
    std::vector<uint8_t> properties_data;
    std::vector<uint8_t> intermediate_data;
    std::vector<uint8_t> final_data;
    if (!converter->ConvertExternalFile(
        std::move(entire_file_data),
        icon_data,
        properties_data,
        intermediate_data,
        final_data))
    {
        return std::nullopt;
    }

    // Write the file
    if (!m_FileWriteCallback(
        taget_path,
        std::move(icon_data),
        std::move(properties_data),
        std::move(entire_file_data_copy),
        std::move(intermediate_data),
        std::move(final_data),
        {},
        static_cast<FileWriteFlags>(FileWriteFlagBits::Overwrite)))
    {
        return std::nullopt;
    }

    return taget_path;
}