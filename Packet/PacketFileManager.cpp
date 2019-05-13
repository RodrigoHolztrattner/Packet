////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileManager.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileManager.h"
#include "PacketFileIndexer.h"
#include "PacketPlainFileIndexer.h"
// #include "PacketCondensedFileIndexer.h"
#include "PacketFileLoader.h"
#include "PacketPlainFileLoader.h"
#include "PacketCondensedFileLoader.h"
#include "PacketFileSaver.h"
#include "PacketFileConverter.h"
#include "PacketFileImporter.h"
#include "PacketReferenceManager.h"
#include "File/PacketFileHeader.h"
#include "File/PacketFileReferences.h"
#include "File/PacketFile.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketFileManager::PacketFileManager(OperationMode _operation_mode, std::wstring _resource_path) :
    m_OperationMode(_operation_mode), 
    m_PacketPath(_resource_path)
{
	// Set the initial data
	// ...
}

PacketFileManager::~PacketFileManager()
{
}

bool PacketFileManager::Initialize()
{
    using namespace std::placeholders;

    // Create our file management objects
    m_FileIndexer          = m_OperationMode == OperationMode::Condensed ? std::make_unique<PacketPlainFileIndexer>(m_PacketPath) : std::make_unique<PacketPlainFileIndexer>(m_PacketPath);
    m_FileLoader           = m_OperationMode == OperationMode::Condensed ? std::make_unique<PacketPlainFileLoader>(*m_FileIndexer) : std::make_unique<PacketPlainFileLoader>(*m_FileIndexer); // PacketCondensedFileLoader
    m_FileReferenceManager = std::make_unique<PacketReferenceManager>();
    m_FileSaver            = std::make_unique<PacketFileSaver>(*m_FileIndexer, *m_FileReferenceManager, *m_FileLoader, m_PacketPath);
    m_FileImporter         = std::make_unique<PacketFileImporter>(
        *m_FileIndexer,
        *m_FileLoader,
        *m_FileReferenceManager,
        std::bind(&PacketFileManager::WriteFile, this, _1, _2, _3, _4, _5, _6, _7, _8, _9),
        [&](std::string _file_extension) {return m_Converters.find(_file_extension) != m_Converters.end() ? m_Converters.find(_file_extension)->second.get() : m_DefaultConverter.get(); },
        m_PacketPath);

    // Set the auxiliary object ptrs
    m_FileIndexer->SetAuxiliarObjects(m_FileLoader.get());
    m_FileReferenceManager->SetAuxiliarObjects(m_FileLoader.get(), m_FileSaver.get());

    // Initialize the file indexer
    if (!m_FileIndexer->Initialize())
    {
        return false;
    }

    return true;
}

void PacketFileManager::RegisterFileConverter(std::string _file_extension, std::unique_ptr<PacketFileConverter> _file_converter)
{
    // Register this converter
    m_Converters.insert({ _file_extension, std::move(_file_converter) });
}

bool PacketFileManager::WriteFile(
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
    bool file_already_indexed = m_FileIndexer->IsFileIndexed(Hash(_target_path));
    if (file_already_indexed && !(_write_flags & static_cast<FileWriteFlags>(FileWriteFlagBits::CreateIfInexistent)))
    {
        return false;
    }

    // Generate the file
    auto file = PacketFile::GenerateFileFromData(
        _target_path,
        _file_type,
        std::move(_icon_data),
        std::move(_properties_data),
        std::move(_original_data),
        std::move(_intermediate_data),
        std::move(_final_data),
        std::move(_file_dependencies));
    if (!file)
    {
        return false;
    }

    // Save the file
    if (!m_FileSaver->SaveFile(std::move(file)))
    {
        return false;
    }

    // If this file isn't indexed yet, index it
    if (!file_already_indexed)
    {
        // Insert a new entry on the file plain indexer
        static_cast<PacketPlainFileIndexer*>(m_FileIndexer.get())->InsertFileIndexData(_target_path);
    }

    return true;
}

bool PacketFileManager::CopyFile(Path _source_file_path, Path _target_file_path) const
{
    // Confirm that the source file is valid
    if (!m_FileIndexer->IsFileIndexed(Hash(_source_file_path)))
    {
        return false;
    }

    // Update the target file path with a valid file path if the current one is already in use
    {
        Path updated_target_path = _target_file_path;
        uint32_t counter         = 1;
        while (m_FileIndexer->IsFileIndexed(Hash(updated_target_path)))
        {
            std::string internal_path = _target_file_path.String();
            internal_path += "_" + std::to_string(counter++);
            updated_target_path = internal_path;
        }
        _target_file_path = updated_target_path;
    }

    auto source_file_hash = Hash(_source_file_path);
    auto target_file_hash = Hash(_target_file_path);

    // Check if the file is an external one, if true just do a normal system copy
    // TODO: ...

    // Load the source file
    auto source_file = m_FileLoader->LoadFile(source_file_hash);
    if (!source_file)
    {
        return false;
    }

    // Duplicate the file
    auto duplicated_file = PacketFile::DuplicateFile(source_file);
    if (!duplicated_file)
    {
        return false;
    }

    // Clear the file links
    duplicated_file->ClearFileLinks();

    // Save the file
    if (!m_FileSaver->SaveFile(std::move(duplicated_file)))
    {
        return false;
    }

    // Insert a new entry on the file plain indexer
    static_cast<PacketPlainFileIndexer*>(m_FileIndexer.get())->InsertFileIndexData(_target_file_path);

    return true;
}

bool PacketFileManager::MoveFile(Path _source_file_path, Path _target_file_path) const
{
    // Confirm that the source file is valid
    if (!m_FileIndexer->IsFileIndexed(Hash(_source_file_path)))
    {
        return false;
    }

    // Update the target file path with a valid file path if the current one is already in use
    {
        Path updated_target_path = _target_file_path;
        uint32_t counter = 1;
        while (m_FileIndexer->IsFileIndexed(Hash(updated_target_path)))
        {
            std::string internal_path = _target_file_path.String();
            internal_path += "_" + std::to_string(counter++);
            updated_target_path = internal_path;
        }
        _target_file_path = updated_target_path;
    }

    auto source_file_hash = Hash(_source_file_path);
    auto target_file_hash = Hash(_target_file_path);

    // Check if the file is an external one, if true just do a normal system move
    // TODO: ...

    // Load the source file
    auto source_file = m_FileLoader->LoadFile(source_file_hash);
    if (!source_file)
    {
        return false;
    }

    // Duplicate the file
    auto duplicated_file = PacketFile::DuplicateFile(source_file);
    if (!duplicated_file)
    {
        return false;
    }

    // Save the file
    if (!m_FileSaver->SaveFile(std::move(duplicated_file)))
    {
        return false;
    }

    // Delete the old file
    if (!DeleteFile(_source_file_path))
    {
        return false;
    }

    // Insert a new entry on the file plain indexer
    static_cast<PacketPlainFileIndexer*>(m_FileIndexer.get())->InsertFileIndexData(_target_file_path);

    // Delete the old entry from the file plain indexer
    static_cast<PacketPlainFileIndexer*>(m_FileIndexer.get())->RemoveFileIndexData(_source_file_path);

    return true;
}

bool PacketFileManager::DeleteFile(Path _target_file_path) const
{
    // Confirm that the source file is valid
    if (!m_FileIndexer->IsFileIndexed(Hash(_target_file_path)))
    {
        return false;
    }

    // Determine the system path this file is located
    auto file_sysytem_path = MergeSystemPathWithFilePath(m_PacketPath, _target_file_path);
    if (!std::filesystem::exists(file_sysytem_path))
    {
        return false;
    }

    // Delete the file
    if (!std::filesystem::remove(file_sysytem_path))
    {
        return false;
    }

    // Delete the old entry from the file plain indexer
    static_cast<PacketPlainFileIndexer*>(m_FileIndexer.get())->RemoveFileIndexData(_target_file_path);

    return true;
}

bool PacketFileManager::WriteFileDataIntoInternalFile(Path _file_path, std::vector<uint8_t>&& _file_data) const
{
    // Determine the filesystem path that the file should be written
    auto file_system_path = MergeSystemPathWithFilePath(m_PacketPath, _file_path);

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