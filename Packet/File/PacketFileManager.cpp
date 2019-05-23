////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileManager.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileManager.h"
#include "Indexer/PacketFileIndexer.h"
#include "Indexer/PacketPlainFileIndexer.h"
#include "Loader/PacketFileLoader.h"
#include "Loader/PacketPlainFileLoader.h"
#include "Loader/PacketCondensedFileLoader.h"
#include "Saver/PacketFileSaver.h"
#include "Converter/PacketFileConverter.h"
#include "Converter/PacketFileDefaultConverter.h"
#include "Importer/PacketFileImporter.h"
#include "Backup/PacketBackupManager.h"
#include "Reference/PacketReferenceManager.h"
#include "Reference/PacketFileReferences.h"
#include "PacketFileHeader.h"
#include "PacketFile.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketFileManager::PacketFileManager(OperationMode         _operation_mode,
                                     BackupFlags           _backup_flags,
                                     std::filesystem::path _packet_path,
                                     std::filesystem::path _backup_path) :
    m_OperationMode(_operation_mode), 
    m_PacketPath(_packet_path), 
    m_BackupPath(_backup_path), 
    m_BackupFlags(_backup_flags)
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
    // TODO: This need some urgently organization!
    m_BackupManager        = std::make_unique<PacketBackupManager>(m_PacketPath, m_BackupPath);
    m_DefaultConverter     = std::make_unique<PacketFileDefaultConverter>();
    m_FileIndexer          = m_OperationMode == OperationMode::Condensed ? std::make_unique<PacketPlainFileIndexer>(m_PacketPath) : std::make_unique<PacketPlainFileIndexer>(m_PacketPath);
    m_FileLoader           = m_OperationMode == OperationMode::Condensed ? std::make_unique<PacketPlainFileLoader>(*m_FileIndexer, m_PacketPath) : std::make_unique<PacketPlainFileLoader>(*m_FileIndexer, m_PacketPath); // PacketCondensedFileLoader
    m_FileReferenceManager = std::make_unique<PacketReferenceManager>();
    m_FileSaver            = std::make_unique<PacketFileSaver>(*m_FileIndexer, *m_FileReferenceManager, *m_FileLoader, *m_BackupManager, m_PacketPath, m_BackupFlags & BackupFlags::BackupBeforeOperation);
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

    // Check if we should perform a backup of all indexed files
    if (m_OperationMode == OperationMode::Plain && m_BackupFlags & BackupFlags::BackupOnStartup)
    {
        // Retrieve all indexed files and backup each one
        auto all_indexed_file_paths = m_FileIndexer->GetAllIndexedFiles();
        for (auto& indexed_file_path : all_indexed_file_paths)
        {
            m_BackupManager->BackupFile(indexed_file_path);
        }
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
    // Check if this operation is supported on the current mode
    if (m_OperationMode == OperationMode::Condensed)
    {
        return false;
    }

    // Check if the file extension is correct
    if (_target_path.path().extension() != PacketExtension)
    {
        return false;
    }

    // Clear the affected files for the file saver before we start an operation
    m_FileSaver->ClearAffectedFiles();

    // Check if we already have this file imported
    bool file_already_indexed = m_FileIndexer->IsFileIndexed(Hash(_target_path));
    if (file_already_indexed && !(_write_flags & static_cast<FileWriteFlags>(FileWriteFlagBits::Overwrite)))
    {
        return false;
    }

    // Check if we should ignore missing dependencies
    if (!(_write_flags & static_cast<FileWriteFlags>(FileWriteFlagBits::IgnoreMissingDependencies)))
    {
        // Check if all dependencies are valid
        for (auto& file_dependency : _file_dependencies)
        {
            if (!m_FileIndexer->IsFileIndexed(Hash(file_dependency)))
            {
                return false;
            }

            if (m_FileIndexer->IsFileExternal(Hash(file_dependency)))
            {
                return false;
            }
        }
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
    if (!m_FileSaver->SaveFile(std::move(file), file_already_indexed ? SaveOperation::Overwrite : SaveOperation::Create))
    {
        return false;
    }

    // Insert a new entry or update an existing one on the file indexer
    static_cast<PacketPlainFileIndexer*>(m_FileIndexer.get())->InsertFileIndexData(_target_path);

    return true;
}

std::optional<Path> PacketFileManager::CopyFile(Path _source_file_path, Path _target_file_dir) const
{
    // Check if this operation is supported on the current mode
    if (m_OperationMode == OperationMode::Condensed)
    {
        return std::nullopt;
    }

    // Clear the affected files for the file saver before we start an operation
    m_FileSaver->ClearAffectedFiles();

    // Confirm that the source file is valid
    if (!m_FileIndexer->IsFileIndexed(Hash(_source_file_path)))
    {
        return std::nullopt;
    }

    // Transform to system path and get all necessary info
    auto source_file_system_path = std::filesystem::path(_source_file_path.string());
    auto [target_directory_system_path, required_file_name, required_file_extension] = DetermineRequiredPaths(_source_file_path, _target_file_dir);

    // Update the target file path with a valid file path if the current one is already in use
    Path target_file_path = RetrieveValidFilePath(target_directory_system_path.string(), required_file_name, required_file_extension);
    auto source_file_hash = Hash(_source_file_path);
    auto target_file_hash = Hash(target_file_path);

    // Load the source file
    auto source_file = m_FileLoader->LoadFile(source_file_hash);
    if (!source_file)
    {
        SignalOperationError("CopyFile", "Failed to load source file");
        return std::nullopt;
    }

    // Check if the source file is an external file, if true just perform a normal copy
    if (source_file->IsExternalFile())
    {
        // Setup the filesystem paths
        auto system_source_path = MergeSystemPathWithFilePath(m_PacketPath, _source_file_path);
        auto system_target_path = MergeSystemPathWithFilePath(m_PacketPath, target_file_path);

        // Perform the copy, also there is no need to perform a backup since we are copying a file,
        // so it's expected that the target file doesn't exist
        std::error_code error;
        std::filesystem::copy(system_source_path, system_target_path, std::filesystem::copy_options::overwrite_existing, error);
        if (error)
        {
            SignalOperationError("CopyFile", "Failed to copy external file");
            return std::nullopt;
        }
    }
    else
    {
        // Duplicate the file
        auto duplicated_file = PacketFile::DuplicateFile(source_file);
        if (!duplicated_file)
        {
            SignalOperationError("CopyFile", "Failed to duplicated source file");
            return std::nullopt;
        }

        // Update the duplicated file path
        duplicated_file->UpdateFilePath(target_file_path);

        // Clear the file links
        duplicated_file->ClearFileLinks();

        // Save the file
        if (!m_FileSaver->SaveFile(std::move(duplicated_file), SaveOperation::Copy))
        {
            SignalOperationError("CopyFile", "Failed to save copied file");
            return std::nullopt;
        }
    }

    // Insert a new entry on the file plain indexer
    static_cast<PacketPlainFileIndexer*>(m_FileIndexer.get())->InsertFileIndexData(target_file_path);

    return target_file_path;
}

std::optional<Path> PacketFileManager::MoveFile(Path _source_file_path, Path _target_file_dir) const
{
    // Check if this operation is supported on the current mode
    if (m_OperationMode == OperationMode::Condensed)
    {
        return std::nullopt;
    }

    // Clear the affected files for the file saver before we start an operation
    m_FileSaver->ClearAffectedFiles();

    // Confirm that the source file is valid
    if (!m_FileIndexer->IsFileIndexed(Hash(_source_file_path)))
    {
        return std::nullopt;
    }

    // Transform to system path and get all necessary info
    auto source_file_system_path = std::filesystem::path(_source_file_path.string());
    auto [target_directory_system_path, required_file_name, required_file_extension] = DetermineRequiredPaths(_source_file_path, _target_file_dir);

    // If the resource and target directories are the same, we don't need to do anything
    if (source_file_system_path.parent_path() == target_directory_system_path)
    {
        return _source_file_path;
    }

    // Update the target file path with a valid file path if the current one is already in use
    Path target_file_path = RetrieveValidFilePath(target_directory_system_path.string(), required_file_name, required_file_extension);
    auto source_file_hash = Hash(_source_file_path);
    auto target_file_hash = Hash(target_file_path);

    // Load the source file
    auto source_file = m_FileLoader->LoadFile(source_file_hash);
    if (!source_file)
    {
        SignalOperationError("MoveFile", "Failed to load source file");
        return std::nullopt;
    }

    // Check if the source file is an external file, if true just perform a normal move
    if (source_file->IsExternalFile())
    {
        // Setup the filesystem paths
        auto system_source_path = MergeSystemPathWithFilePath(m_PacketPath, _source_file_path);
        auto system_target_path = MergeSystemPathWithFilePath(m_PacketPath, target_file_path);

        // Perform the copy, also there is no need to perform a backup since we are copying a file,
        // so it's expected that the target file doesn't exist
        std::error_code error;
        std::filesystem::rename(system_source_path, system_target_path, error);
        if (error)
        {
            SignalOperationError("MoveFile", "Failed to move external file");
            return std::nullopt;
        }
    }
    else
    {
        // Duplicate the file
        auto duplicated_file = PacketFile::DuplicateFile(source_file);
        if (!duplicated_file)
        {
            SignalOperationError("MoveFile", "Failed to duplicate source file");
            return std::nullopt;
        }

        // Update the duplicated file path
        duplicated_file->UpdateFilePath(target_file_path);

        // Save the file
        if (!m_FileSaver->SaveFile(std::move(duplicated_file), SaveOperation::Move))
        {
            SignalOperationError("MoveFile", "Failed to save duplicated file");
            return std::nullopt;
        }

        // Delete the old file
        if (!DeleteFile(_source_file_path, false))
        {
            SignalOperationError("MoveFile", "Failed to delete old file");
            return std::nullopt;
        }
    }

    // Insert a new entry on the file plain indexer
    static_cast<PacketPlainFileIndexer*>(m_FileIndexer.get())->InsertFileIndexData(target_file_path);

    // Delete the old entry from the file plain indexer
    static_cast<PacketPlainFileIndexer*>(m_FileIndexer.get())->RemoveFileIndexData(_source_file_path);

    return target_file_path;
}

std::optional<Path> PacketFileManager::RenameFile(Path _source_file_path, Path _new_file_name) const
{
    // Check if this operation is supported on the current mode
    if (m_OperationMode == OperationMode::Condensed)
    {
        return std::nullopt;
    }

    // Clear the affected files for the file saver before we start an operation
    m_FileSaver->ClearAffectedFiles();

    // Confirm that the source file is valid
    if (!m_FileIndexer->IsFileIndexed(Hash(_source_file_path)))
    {
        return std::nullopt;
    }

    // Check if the filename is the same
    if (_source_file_path.path().stem() == _new_file_name.path())
    {
        return _source_file_path;
    }

    // Determine the new path for this file
    auto new_file_path = _source_file_path.path().parent_path().string() + "/" + _new_file_name.string() + _source_file_path.path().extension().string();

    // Confirm that the requested name is available
    if (m_FileIndexer->IsFileIndexed(Hash(new_file_path)))
    {
        return std::nullopt;
    }

    auto source_file_hash = Hash(_source_file_path);
    auto target_file_hash = Hash(new_file_path);

    // Load the source file
    auto source_file = m_FileLoader->LoadFile(source_file_hash);
    if (!source_file)
    {
        SignalOperationError("RenameFile", "Failed to load source file");
        return std::nullopt;
    }

    // Check if the source file is an external file, if true just perform a normal rename
    if (source_file->IsExternalFile())
    {
        // Setup the filesystem paths
        auto system_source_path = MergeSystemPathWithFilePath(m_PacketPath, _source_file_path);
        auto system_target_path = MergeSystemPathWithFilePath(m_PacketPath, new_file_path);

        // Perform the copy, also there is no need to perform a backup since we are copying a file,
        // so it's expected that the target file doesn't exist
        std::error_code error;
        std::filesystem::rename(system_source_path, system_target_path, error);
        if (error)
        {
            SignalOperationError("RenameFile", "Failed to rename external file");
            return std::nullopt;
        }
    }
    else
    {
        // Update the file path
        source_file->UpdateFilePath(new_file_path);

        // Save the file
        if (!m_FileSaver->SaveFile(std::move(source_file), SaveOperation::Move))
        {
            SignalOperationError("RenameFile", "Failed to save source file");
            return std::nullopt;
        }

        // Delete the old file
        if (!DeleteFile(_source_file_path, false))
        {
            SignalOperationError("RenameFile", "Failed to delete old file");
            return std::nullopt;
        }
    }

    // Insert a new entry on the file plain indexer
    static_cast<PacketPlainFileIndexer*>(m_FileIndexer.get())->InsertFileIndexData(new_file_path);

    // Delete the old entry from the file plain indexer
    static_cast<PacketPlainFileIndexer*>(m_FileIndexer.get())->RemoveFileIndexData(_source_file_path);

    return new_file_path;
}

bool PacketFileManager::RedirectFileDependencies(Path _source_file_path, Path _target_file_path)
{
    // Check if this operation is supported on the current mode
    if (m_OperationMode == OperationMode::Condensed)
    {
        return false;
    }

    // Clear the affected files for the file saver before we start an operation
    m_FileSaver->ClearAffectedFiles();

    // Confirm that the source and target files are valid
    if (!m_FileIndexer->IsFileIndexed(Hash(_source_file_path)) || !m_FileIndexer->IsFileIndexed(Hash(_target_file_path)))
    {
        return false;
    }

    // Load the source file
    auto source_file = m_FileLoader->LoadFile(Hash(_source_file_path));
    if (!source_file)
    {
        SignalOperationError("RedirectFileDependencies", "Failed to load source file");
        return false;
    }

    // Load the target file
    auto target_file = m_FileLoader->LoadFile(Hash(_target_file_path));
    if (!target_file)
    {
        SignalOperationError("RedirectFileDependencies", "Failed to load target file");
        return false;
    }

    // Check if both files aren't external
    if (source_file->IsExternalFile() || target_file->IsExternalFile())
    {
        SignalOperationError("RedirectFileDependencies", "Source and/or target file is invalid (external)");
        return false;
    }

    // Check if both files are compatible
    if (source_file->GetFileHeader().GetFileType() != target_file->GetFileHeader().GetFileType())
    {
        SignalOperationError("RedirectFileDependencies", "Source and target files are incompatible");
        return false;
    }

    // TODO: Verify recursive/infinite dependency
    // ...

    // Make the redirection
    {
        // For each file that used to depend on the source one, modify all references on it to point to
        // the target one
        if (!m_FileReferenceManager->SubstituteDependencyReferences(source_file->GetFileReferences().GetFileLinks(), source_file->GetFileHeader().GetPath(), _target_file_path))
        {
            SignalOperationError("RedirectFileDependencies", "Failed to substitute dependency references");
            return false;
        }

        // Now the target resource must acknowledge all new resources that depends on it, lets add all the 
        // source file links to it
        if (!m_FileReferenceManager->AddReferenceLink(source_file->GetFileReferences().GetFileLinks(), _target_file_path))
        {
            SignalOperationError("RedirectFileDependencies", "Failed to add reference links");
            return false;
        }

        // Clear the file links from the source file since it doesn't have them anymore
        source_file->ClearFileLinks();
    }

    // Save the file
    if (!m_FileSaver->SaveFile(std::move(source_file), SaveOperation::Overwrite))
    {
        SignalOperationError("RedirectFileDependencies", "Failed to save source file");
        return false;
    }

    return true;
}

bool PacketFileManager::DeleteFile(Path _target_file_path) const
{
    // Check if this operation is supported on the current mode
    if (m_OperationMode == OperationMode::Condensed)
    {
        return false;
    }

    // Clear the affected files for the file saver before we start an operation
    m_FileSaver->ClearAffectedFiles();

    return DeleteFile(_target_file_path, true);
}

void PacketFileManager::RegisterOperationFailureCallback(std::function<void(std::string, std::string, const PacketBackupManager& _backup_manager, std::set<Path>)> _callback)
{
    m_OperationFailureCallback = _callback;
}

bool PacketFileManager::DeleteFile(Path _target_file_path, bool _remove_dependency_links) const
{
    // Confirm that the source file is valid
    if (!m_FileIndexer->IsFileIndexed(Hash(_target_file_path)))
    {
        return false;
    }

    // If enabled, perform a backup of this file
    if (m_BackupFlags & BackupFlags::BackupBeforeOperation)
    {
        m_BackupManager->BackupFile(_target_file_path);
    }

    // Determine the system path this file is located
    auto file_sysytem_path = MergeSystemPathWithFilePath(m_PacketPath, _target_file_path);
    if (!std::filesystem::exists(file_sysytem_path))
    {
        return false;
    }

    // If this file dependency links must be removed
    if (_remove_dependency_links)
    {
        // Load the file references
        auto file_references_data_opt = m_FileLoader->LoadFileDataPart(Hash(_target_file_path), FilePart::ReferencesData);
        if (!file_references_data_opt)
        {
            return false;
        }
        auto [header, file_references_data] = file_references_data_opt.value();
        auto file_references = PacketFileReferences::CreateFromData(file_references_data);

        // Remove all dependency links that this file had on other files
        for (auto& dependency : file_references.GetFileDependencies())
        {
            m_FileReferenceManager->RemoveReferenceLink(header.GetPath(), dependency);
        }
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

std::tuple<std::filesystem::path, std::string, std::string> PacketFileManager::DetermineRequiredPaths(
    Path _source_file_path,
    Path _target_path) const
{
    // Determine if the target path contains a file or a directory
    if (std::filesystem::is_directory(MergeSystemPathWithFilePath(m_PacketPath, _target_path)))
    {
        // Transform to system path and get all necessary info
        auto source_file_system_path = std::filesystem::path(_source_file_path.string());
        auto target_directory_system_path = std::filesystem::path(_target_path.string());
        auto required_file_name = source_file_system_path.stem().string();
        auto required_file_extension = source_file_system_path.extension().string();

        return { target_directory_system_path, required_file_name, required_file_extension };
    }
    else
    {
        // Transform to system path and get all necessary info
        auto source_file_system_path = std::filesystem::path(_source_file_path.string());
        auto target_file_system_path = std::filesystem::path(_target_path.string());
        auto target_directory_system_path = std::filesystem::path(_target_path.string()).parent_path();
        auto required_file_name = target_file_system_path.stem().string();
        auto required_file_extension = target_file_system_path.extension().string();

        return { target_directory_system_path, required_file_name, required_file_extension };
    }
}

Path PacketFileManager::RetrieveValidFilePath(const std::filesystem::path& _directory_path, const std::string& _file_name, const std::string& _file_extension) const
{
    // Update the target file path with a valid file path if the current one is already in use
    Path target_file_path = _directory_path.string() + "/" + _file_name + _file_extension;
    {
        Path updated_target_path = target_file_path;
        uint32_t counter = 1;
        while (m_FileIndexer->IsFileIndexed(Hash(updated_target_path)))
        {
            std::string internal_path = _file_name + "_" + std::to_string(counter++);
            updated_target_path = _directory_path.string() + "/" + internal_path + _file_extension;
        }
        target_file_path = updated_target_path;
    }

    return target_file_path;
}

void PacketFileManager::SignalOperationError(std::string _operation, std::string _error_message) const
{
    // Check if any file was affected
    auto affected_files = m_FileSaver->GetAffectedFiles();
    bool files_were_affected = affected_files.size() != 0;

    // Verify if the backups for the affected files should be automatically restored
    if (m_BackupFlags & BackupFlags::AutomaticallyRestoreOnOperationFailure)
    {
        for (auto& affected_file_path : affected_files)
        {
            m_BackupManager->RestoreFile(affected_file_path);
        }
    }

    // If the callback was set, call it
    if (m_OperationFailureCallback)
    {
        m_OperationFailureCallback(_operation, _error_message, *m_BackupManager, affected_files);
    }
}

PacketFileIndexer& PacketFileManager::GetFileIndexer() const
{
    return *m_FileIndexer;
}

const PacketFileLoader& PacketFileManager::GetFileLoader() const
{
    return *m_FileLoader;
}

const PacketFileImporter& PacketFileManager::GetFileImporter() const
{
    return *m_FileImporter;
}