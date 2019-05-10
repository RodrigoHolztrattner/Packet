////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileImporter.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileImporter.h"
#include "PacketPlainFileIndexer.h"
#include "PacketFileIndexer.h"
#include "PacketFileLoader.h"
#include "PacketReferenceManager.h"
#include "PacketFile.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketFileImporter::PacketFileImporter(
    PacketFileIndexer& _file_indexer,
    const PacketFileLoader& _file_loader,
    const PacketReferenceManager& _reference_manager,
    std::wstring _resource_path) :
    m_FileIndexer(_file_indexer),
    m_FileLoader(_file_loader), 
    m_ReferenceManager(_reference_manager), 
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
    // TODO: Create the default converter
    // ...

    return true;
}

void PacketFileImporter::RegisterFileConverter(std::string _file_extension, std::unique_ptr<PacketFileConverter> _file_converter)
{
    // Register this converter
    m_Converters.insert({ _file_extension, std::move(_file_converter) });
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
    PacketFileConverter* converter = nullptr;
    {
        auto iter = m_Converters.find(file_extension);
        converter = (iter == m_Converters.end() ? m_DefaultConverter.get() : iter->second.get());
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
    if (!WriteInternalFile(
        _target_path,
        converter->GetConversionFileType(),
        std::move(icon_data),
        std::move(properties_data),
        std::move(entire_file_data_copy), 
        std::move(intermediate_data),
        std::move(final_data),
        {}), 
        static_cast<FileWriteFlags>(FileWriteFlagBits::CreateIfInexistent))
    {
        return false;
    }
}

bool PacketFileImporter::WriteInternalFile(
    Path                   _target_path,
    FileType               _file_type,
    std::vector<uint8_t>&& _icon_data,
    std::vector<uint8_t>&& _properties_data,
    std::vector<uint8_t>&& _original_data,
    std::vector<uint8_t>&& _intermediate_data,
    std::vector<uint8_t>&& _final_data,
    std::set<Path>&&       _file_dependencies,
    FileWriteFlags         _write_flags) const
{
    // Check if we already have this file imported
    bool file_already_indexed = m_FileIndexer.IsFileIndexed(Hash(_target_path));
    if (file_already_indexed && !(_write_flags & static_cast<FileWriteFlags>(FileWriteFlagBits::CreateIfInexistent)))
    {
        return false;
    }

    // Retrieve the current file data
    std::set<Path> file_links;
    auto current_file_data = m_FileLoader.LoadFile(Hash(_target_path));
    if (current_file_data)
    {
        // Get the current references for this file
        auto& file_references = current_file_data->GetFileReferences();

        // Get the dependencies
        auto& file_current_dependencies = file_references.GetFileDependencies();

        // Set the links
        file_links = file_references.GetFileLinks();

        // Check if the dependencies are equal, if true do nothing
        if (_file_dependencies == file_current_dependencies)
        {
            // Do nothing
        }
        // New dependencies > old dependencies
        else if (_file_dependencies.size() > file_current_dependencies.size())
        {
            std::set<Path> diff;
            std::set_difference(
                _file_dependencies.begin(),
                _file_dependencies.end(),
                file_current_dependencies.begin(),
                file_current_dependencies.end(),
                diff);

            // Add the references
            for (auto& new_entry : diff)
            {
                m_ReferenceManager.AddReferenceLink(new_entry, _target_path);
            }
        }
        // New dependencies < old dependencies
        else
        {
            std::set<Path> diff;
            std::set_difference(
                file_current_dependencies.begin(),
                file_current_dependencies.end(),
                _file_dependencies.begin(),
                _file_dependencies.end(),
                diff);

            // Remove the references
            for (auto& entry : diff)
            {
                m_ReferenceManager.RemoveReferenceLink(entry, _target_path);
            }
        }
    }

    // Generate the internal file data
    auto file_data = PacketFile::GenerateInternalFileData(
        Hash(_target_path),
        _file_type,
        std::move(_icon_data),
        std::move(_properties_data),
        std::move(_original_data),
        std::move(_intermediate_data),
        std::move(_final_data),
        std::move(file_links),
        std::move(_file_dependencies));
    if (file_data.size() == 0)
    {
        return false;
    }

    // Write the file data into a system file
    if (!WriteFileDataIntoInternalFile(_target_path, std::move(file_data)))
    {
        return false;
    }

    // Insert a new entry on the file plain indexer
    static_cast<PacketPlainFileIndexer&>(m_FileIndexer).InsertFileIndexData(_target_path);
}

bool PacketFileImporter::CopyInternalFile(Path _source_file_path, Path _target_file_path) const
{
    // Confirm that the source file is valid
    if (!m_FileIndexer.IsFileIndexed(Hash(_source_file_path)))
    {
        return false;
    }

    // Update the target file path with a valid file path if the current one is already in use
    {
        Path updated_target_path = _target_file_path;
        uint32_t counter         = 1;
        while (m_FileIndexer.IsFileIndexed(Hash(updated_target_path)))
        {
            std::string internal_path = _target_file_path.String();
            internal_path += "_" + std::to_string(counter++);
            updated_target_path = internal_path;
        }
        _target_file_path = updated_target_path;
    }

    auto source_file_hash = Hash(_source_file_path);
    auto target_file_hash = Hash(_target_file_path);

    // Load the source file
    auto source_file_data = m_FileLoader.LoadFile(source_file_hash);
    if (!source_file_data)
    {
        return false;
    }

    // Get the source file internal data
    std::vector<uint8_t> icon_data;
    std::vector<uint8_t> properties_data;
    std::vector<uint8_t> original_data;
    std::vector<uint8_t> intermediate_data;
    std::vector<uint8_t> final_data;
    PacketFileReferences file_references_parsed;
    if (!PacketFile::BreakFileIntoDatas(
        std::move(source_file_data),
        icon_data,
        properties_data,
        original_data,
        intermediate_data,
        final_data,
        file_references_parsed))
    {
        return false;
    }

    auto file_dependencies = file_references_parsed.GetFileDependencies();

    // Write the new file
    if (!WriteInternalFile(
        _target_file_path,
        source_file_data->GetFileHeader().file_type,
        std::move(icon_data),
        std::move(properties_data),
        std::move(original_data),
        std::move(intermediate_data),
        std::move(final_data),
        {}))
    {
        return false;
    }

    return true;
}

bool PacketFileImporter::MoveInternalFile(Path _source_file_path, Path _target_file_path) const
{
    // Confirm that the source file is valid
    if (!m_FileIndexer.IsFileIndexed(Hash(_source_file_path)))
    {
        return false;
    }

    // Update the target file path with a valid file path if the current one is already in use
    {
        Path updated_target_path = _target_file_path;
        uint32_t counter = 1;
        while (m_FileIndexer.IsFileIndexed(Hash(updated_target_path)))
        {
            std::string internal_path = _target_file_path.String();
            internal_path += "_" + std::to_string(counter++);
            updated_target_path = internal_path;
        }
        _target_file_path = updated_target_path;
    }

    auto source_file_hash = Hash(_source_file_path);
    auto target_file_hash = Hash(_target_file_path);

    // Load the source file
    auto source_file_data = m_FileLoader.LoadFile(source_file_hash);
    if (!source_file_data)
    {
        return false;
    }

    // Get the source file internal data
    std::vector<uint8_t> icon_data;
    std::vector<uint8_t> properties_data;
    std::vector<uint8_t> original_data;
    std::vector<uint8_t> intermediate_data;
    std::vector<uint8_t> final_data;
    PacketFileReferences file_references_parsed;
    if (!PacketFile::BreakFileIntoDatas(
        std::move(source_file_data),
        icon_data,
        properties_data,
        original_data,
        intermediate_data,
        final_data,
        file_references_parsed))
    {
        return false;
    }

    auto file_dependencies = file_references_parsed.GetFileDependencies();

    // Write the new file
    if (!WriteInternalFile(
        _target_file_path,
        source_file_data->GetFileHeader().file_type,
        std::move(icon_data),
        std::move(properties_data),
        std::move(original_data),
        std::move(intermediate_data),
        std::move(final_data),
        std::move(file_dependencies)))
    {
        return false;
    }

    return true;
}

bool PacketFileImporter::WriteFileDataIntoInternalFile(Path _file_path, std::vector<uint8_t>&& _file_data) const
{
    // Determine the filesystem path that the file should be written
    auto file_system_path = MergeSystemPathWithFilePath(m_ResourcePath, _file_path);

    // Open the file and check if we are ok to proceed
    std::ofstream file(file_system_path, std::ios::binary);
    if (!file.is_open())
    {
        // Error opening the file!
        return false;
    }

    file.write(reinterpret_cast<const char*>(_file_data.data()), _file_data.size() * sizeof(uint8_t));

    // Close the file
    file.close();

    return true;
}