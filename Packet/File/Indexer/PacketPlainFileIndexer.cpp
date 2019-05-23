////////////////////////////////////////////////////////////////////////////////
// Filename: PacketPlainFileIndexer.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketPlainFileIndexer.h"
#include "../Loader/PacketFileLoader.h"
#include "../Watcher/FileWatcherManager.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketPlainFileIndexer::PacketPlainFileIndexer(std::filesystem::path _packet_path) : 
    PacketFileIndexer(_packet_path)
{
	// Set the initial data
    m_FileWatcherManager = std::make_unique<FileWatcherManager>(true);

    // Register the file watcher callback
    m_FileWatcherManager->RegisterOnFileDataChangedMethod(
        [&](Path _file_path) {

            // Call all registered callbacks since this file was modified/added
            for (auto& callback : m_FileModificationCallbacks)
            {
                callback(_file_path);
            }
        });
}

PacketPlainFileIndexer::~PacketPlainFileIndexer()
{
}

bool PacketPlainFileIndexer::Initialize()
{
    // Scan the resource path
    BuildFilesystemView(m_PacketPath);

    return true;
}

bool PacketPlainFileIndexer::IsFileIndexed(HashPrimitive _file_hash) const
{
    std::shared_lock lock(m_Mutex);

    return m_IndexDatas.find(_file_hash) != m_IndexDatas.end();
}

std::optional<PacketFileIndexer::FileLoadInformation> PacketPlainFileIndexer::RetrieveFileLoadInformation(HashPrimitive _file_hash) const
{
    std::shared_lock lock(m_Mutex);

    auto iter = m_IndexDatas.find(_file_hash);
    if (iter != m_IndexDatas.end())
    {
        return m_IndexDatas.find(_file_hash)->second.file_load_information;
    }

    return std::nullopt;
}

std::optional<std::string> PacketPlainFileIndexer::GetFileExtension(HashPrimitive _file_hash) const
{
    std::shared_lock lock(m_Mutex);

    auto iter = m_IndexDatas.find(_file_hash);
    if (iter != m_IndexDatas.end())
    {
        return m_IndexDatas.find(_file_hash)->second.file_extension;
    }

    return std::nullopt;
}

std::optional<PacketFileHeader> PacketPlainFileIndexer::GetFileHeader(HashPrimitive _file_hash) const
{
    std::shared_lock lock(m_Mutex);

    auto iter = m_IndexDatas.find(_file_hash);
    if (iter != m_IndexDatas.end())
    {
        return m_IndexDatas.find(_file_hash)->second.file_header;
    }

    return std::nullopt;
}

std::vector<uint8_t> PacketPlainFileIndexer::GetFileIconData(HashPrimitive _file_hash) const
{
    std::shared_lock lock(m_Mutex);

    auto iter = m_IndexDatas.find(_file_hash);
    if (iter != m_IndexDatas.end())
    {
        return m_IndexDatas.find(_file_hash)->second.icon_data;
    }

    return std::vector<uint8_t>();
}

nlohmann::json PacketPlainFileIndexer::GetFileProperties(HashPrimitive _file_hash) const
{
    std::shared_lock lock(m_Mutex);

    auto iter = m_IndexDatas.find(_file_hash);
    if (iter != m_IndexDatas.end())
    {
        return m_IndexDatas.find(_file_hash)->second.file_properties;

    }

    return nlohmann::json();
}

bool PacketPlainFileIndexer::IsFileExternal(HashPrimitive _file_hash) const
{
    std::shared_lock lock(m_Mutex);

    auto iter = m_IndexDatas.find(_file_hash);
    if (iter != m_IndexDatas.end())
    {
        return m_IndexDatas.find(_file_hash)->second.file_is_external;
    }

    // Returning true is the safest option on this case if not throwing
    throw "Invalid file hash";
    return true;
}

std::set<Path> PacketPlainFileIndexer::GetAllIndexedFiles() const
{
    std::shared_lock lock(m_Mutex);
    std::set<Path> indexed_paths;

    for (auto& indexed_data : m_IndexDatas)
    {
        indexed_paths.insert(indexed_data.second.file_load_information.file_path);
    }

    return indexed_paths;
}

std::vector<std::pair<Path, std::set<Path>>> PacketPlainFileIndexer::GetMissingDependenciesInfo() const
{
    std::shared_lock lock(m_Mutex);
    std::vector<std::pair<Path, std::set<Path>>> result;

    // For each indexed data
    for (auto& indexed_data_pair : m_IndexDatas)
    {
        const IndexData& indexed_data = indexed_data_pair.second;

        // Setup the entry
        std::pair<Path, std::set<Path>> entry;
        entry = { indexed_data.file_load_information.file_path, {} };

        // For each dependency
        for (auto& file_dependency : indexed_data.file_references.GetFileDependencies())
        {
            // Check if this file is indexed
            if (m_IndexDatas.find(Hash(file_dependency)) == m_IndexDatas.end())
            {
                // Insert into our entry
                entry.second.insert(file_dependency);
            }
        }

        // If we have at least one missing dependency, add the entry into our result
        if (entry.second.size() > 0)
        {
            result.push_back(std::move(entry));
        }
    }

    return result;
}

void PacketPlainFileIndexer::BuildFilesystemView(std::filesystem::path _resource_path)
{
    // This method will recursively construct the packet tree
    std::function<void(const std::filesystem::path&)> ScanResourcePathRecursive = [&](const std::filesystem::path & _current_path)
    {
        // For each folder/file recursively
        for (auto& p : std::filesystem::directory_iterator(_current_path))
        {
            // Get the path
            auto path = std::filesystem::path(p);

            // Check if this is a file
            if (std::filesystem::is_regular_file(p))
            {
                // Gather this resource data and insert it into our index
                InsertFileIndexData(ConvertSystemPathIntoInternalPath(_resource_path, path));
            }
        }
    };

    // Call the scan method for the base resource path
    ScanResourcePathRecursive(_resource_path);
}

void PacketPlainFileIndexer::InsertFileIndexData(Path _file_path)
{
    std::unique_lock lock(m_Mutex);

    // Get the system file info
    auto system_filepath = MergeSystemPathWithFilePath(m_PacketPath, _file_path);

    // Check if the file exist
    if (!std::filesystem::exists(system_filepath))
    {
        return;
    }

    // Our new index entry
    IndexData new_index_entry = {};

    // Load this file to retrieve its header, icon data and properties
    auto file = m_FileLoaderPtr->LoadFile(Hash(_file_path));
    if (file && !file->IsExternalFile())
    {
        // Set the header, icon data and properties
        new_index_entry.file_header = file->GetFileHeader();
        new_index_entry.icon_data = file->GetIconData();
        new_index_entry.file_properties = file->GetFileProperties();
    }

    // If this file is an external type, register a watcher for it to enable us calling
    // the file modification callback if for any reason it is modified
    if (file->IsExternalFile())
    {
        // Register a watcher for this file
        m_FileWatcherManager->RequestWatcher(system_filepath, _file_path);
    }

    // Set the file extension and load data
    new_index_entry.file_extension = system_filepath.extension().string();
    new_index_entry.file_load_information.file_path = _file_path;
    new_index_entry.file_load_information.file_hash = Hash(_file_path);
    new_index_entry.file_load_information.file_data_position = 0;
    new_index_entry.file_load_information.file_data_size = std::filesystem::file_size(system_filepath);
    new_index_entry.file_is_external = file->IsExternalFile();
    new_index_entry.file_references = file->GetFileReferences();

    // Add the entry
    m_IndexDatas.insert({ Hash(_file_path), std::move(new_index_entry) });

    // Call all registered callbacks since this file was modified/added
    for (auto& callback : m_FileModificationCallbacks)
    {
        callback(_file_path);
    }
}

void PacketPlainFileIndexer::RemoveFileIndexData(Path _file_path)
{
    std::unique_lock lock(m_Mutex);

    // Find an entry with this path
    auto iter = m_IndexDatas.find(Hash(_file_path));
    if (iter != m_IndexDatas.end())
    {
        m_IndexDatas.erase(iter);
    }

    // Remove the path watcher that potentially was added to watch this file if
    // it was an external one
    m_FileWatcherManager->ReleaseWatcher(MergeSystemPathWithFilePath(m_PacketPath, _file_path));
}