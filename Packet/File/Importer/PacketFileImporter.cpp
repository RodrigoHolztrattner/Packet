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
    std::wstring                                      _resource_path) :
    m_FileIndexer(_file_indexer),
    m_FileLoader(_file_loader), 
    m_ReferenceManager(_reference_manager), 
    m_FileWriteCallback(_file_write_callback), 
    m_RetrieveConverterForTypeCallback(), 
    m_ResourcePath(_resource_path)
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

bool PacketFileImporter::ImportExternalFile(std::filesystem::path _file_original_path, Path _target_path, FileImportFlags _import_flags) const
{
    // Check if the file exist and is valid
    if (!std::filesystem::exists(_file_original_path) || std::filesystem::is_directory(_file_original_path))
    {
        // Invalid file
        return false;
    }

    // Check if we already have this file imported, if true, check if we must overwrite it
    bool file_already_indexed = m_FileIndexer.IsFileIndexed(Hash(_target_path));
    if (file_already_indexed && !(_import_flags & static_cast<FileImportFlags>(FileImportFlagBits::Overwrite)))
    {
        // The file already exist and we are not overwriting it
        return false;
    }

    // Determine the file original extension
    std::string file_extension = _file_original_path.extension().string();

    // Check if we have a converter for this file extension
    PacketFileConverter* converter = m_RetrieveConverterForTypeCallback(file_extension);
    if (!converter)
    {
        return false;
    }

    // Open the file and check if we are ok to proceed
    std::ifstream file(_file_original_path, std::ios::binary);
    if (!file.is_open())
    {
        // Error opening the file!
        return false;
    }

    // Reserve space for the entire file
    std::vector<uint8_t> entire_file_data(std::filesystem::file_size(_file_original_path));

    // Read the entire file data
    entire_file_data.insert(
        entire_file_data.begin(),
        std::istream_iterator<uint8_t>(file),
        std::istream_iterator<uint8_t>());

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
        return false;
    }

    // Write the file
    if (!m_FileWriteCallback(
        _target_path,
        converter->GetConversionFileType(),
        std::move(icon_data),
        std::move(properties_data),
        std::move(entire_file_data_copy),
        std::move(intermediate_data),
        std::move(final_data),
        {},
        static_cast<FileWriteFlags>(FileWriteFlagBits::CreateIfInexistent)))
    {
        return false;
    }

    return true;
}