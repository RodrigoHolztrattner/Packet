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
    // Save the cache file data
    std::ofstream file(IndexerCacheData, std::ios::binary);
    if (file.is_open())
    {
        nlohmann::json j = nlohmann::json::array();

        // For each index data
        for (auto& [hash, index_data]: m_IndexDatas)
        {
            // If this index data doesn't have a cache, ignore it
            if (!index_data.file_cache)
            {
                continue;
            }

            // Setup the new entry
            nlohmann::json cache_entry;
            cache_entry["file_is_external"] = index_data.file_cache->file_is_external;
            cache_entry["icon_data"] = index_data.file_cache->icon_data;
            cache_entry["file_properties"] = index_data.file_cache->file_properties;
            cache_entry["file_path"] = index_data.file_load_information.file_path.string();

            j.push_back(cache_entry);
        }

        file << std::setw(4) << j << std::endl;
    }
}

bool PacketPlainFileIndexer::Initialize()
{
    // Scan the resource path
    BuildFilesystemView(m_PacketPath);

    // Check if we have a cache file
    std::ifstream file(IndexerCacheData, std::ios::binary);
    if (file.is_open())
    {
        // Gather the cache data
        nlohmann::json j;
        file >> j;

        file.close();

        // For each cache data
        for (auto& entry : j)
        {
            CacheData cache_data;
            std::string file_path;

            entry.at("file_is_external").get_to(cache_data.file_is_external);
            entry.at("icon_data").get_to(cache_data.icon_data);
            cache_data.file_properties = entry.at("file_properties");
            entry.at("file_path").get_to(file_path);

            // If this file path is indexed, set the cache data, we need to make sure the entry
            // exist because the cache could be invalid, so we cannot use it to add a new index
            // entry since this entry could be also invalid, that's the reason we cannot use
            // RegisterFileCacheData() direct (it register a new index entry if inexistent)
            auto iter = m_IndexDatas.find(Hash(file_path));
            if (iter != m_IndexDatas.end())
            {
                RegisterFileCacheData(file_path, cache_data.file_properties, cache_data.icon_data, cache_data.file_is_external);
            }
        }
    }

    return true;
}

bool PacketPlainFileIndexer::IsFileIndexed(HashPrimitive _file_hash) const
{
    std::shared_lock lock(m_Mutex);

    auto iter = m_IndexDatas.find(_file_hash);
    if (iter != m_IndexDatas.end())
    {
        // If this file isn't cached, try to do it
        if (!iter->second.file_cache)
        {
            // Try to load the file and cache its data
            LoadAndCacheFile(iter->second.file_load_information.file_path);
        }

        return true;
    }

    return false;
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

std::vector<uint8_t> PacketPlainFileIndexer::GetFileIconData(HashPrimitive _file_hash) const
{
    std::shared_lock lock(m_Mutex);

    auto file_cache = GetCacheForFileHash(_file_hash);
    if (file_cache)
    {
        return file_cache->icon_data;
    }

    return std::vector<uint8_t>();
}

nlohmann::json PacketPlainFileIndexer::GetFileProperties(HashPrimitive _file_hash) const
{
    std::shared_lock lock(m_Mutex);

    auto file_cache = GetCacheForFileHash(_file_hash);
    if (file_cache)
    {
        return file_cache->file_properties;
    }

    return nlohmann::json();
}

bool PacketPlainFileIndexer::IsFileExternal(HashPrimitive _file_hash) const
{
    std::shared_lock lock(m_Mutex);

    auto file_cache = GetCacheForFileHash(_file_hash);
    if (file_cache)
    {
        return file_cache->file_is_external;
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

std::optional<PacketPlainFileIndexer::CacheData> PacketPlainFileIndexer::GetCacheForFileHash(HashPrimitive _file_hash) const
{
    auto iter = m_IndexDatas.find(_file_hash);
    if (iter != m_IndexDatas.end())
    {
        if (!iter->second.file_cache)
        {
            // Try to load the file and cache its data
            LoadAndCacheFile(iter->second.file_load_information.file_path);
        }

        return iter->second.file_cache;
    }

    return std::nullopt;
}

std::vector<std::pair<Path, std::set<Path>>> PacketPlainFileIndexer::GetMissingDependenciesInfo() const
{
    std::shared_lock lock(m_Mutex);
    std::vector<std::pair<Path, std::set<Path>>> result;
    /*
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
    */
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
                auto path_string = ConvertSystemPathIntoInternalPath(_resource_path, path).string();
                std::replace(path_string.begin(), path_string.end(), '\\', '/');

                // Index this file
                IndexFileFromPath(path_string);
            }
            else
            {
                // Enter this folder and continue searching
                ScanResourcePathRecursive(path);
            }
        }
    };

    // Call the scan method for the base resource path
    ScanResourcePathRecursive(_resource_path);
}

void PacketPlainFileIndexer::RegisterFileCacheData(Path _file_path, nlohmann::json _file_properties, std::vector<uint8_t> _icon_data, bool _file_is_external)
{
    std::unique_lock lock(m_Mutex);

    // Check if this file path is already indexed
    auto iter = m_IndexDatas.find(Hash(_file_path));
    if (iter == m_IndexDatas.end())
    {
        // Try to index this file path
        IndexFileFromPath(_file_path);

        // Check if we are ok to proceed
        iter = m_IndexDatas.find(Hash(_file_path));
        if (iter == m_IndexDatas.end())
        {
            // Unable to index the file from the given path
            return;
        }
    }

    // Get a short reference to this index cache data
    auto& cache_data = iter->second.file_cache;

    // Set the cache data
    cache_data->file_properties = std::move(_file_properties);
    cache_data->icon_data = std::move(_icon_data);
    cache_data->file_is_external = _file_is_external;

    // If this file is an external type, register a watcher for it to enable us calling
    // the file modification callback if for any reason it is modified
    if (_file_is_external)
    {
        // Get the system file info
        auto system_filepath = MergeSystemPathWithFilePath(m_PacketPath, _file_path);

        // Register a watcher for this file
        m_FileWatcherManager->RequestWatcher(system_filepath, _file_path);
    }
}

void PacketPlainFileIndexer::LoadAndCacheFile(Path _file_path) const
{
    // Get the system file info
    auto system_filepath = MergeSystemPathWithFilePath(m_PacketPath, _file_path);

    // Check if the file exist
    if (!std::filesystem::exists(system_filepath))
    {
        return;
    }

    // Load this file to retrieve its cache data
    CacheData file_cache;
    auto file = m_FileLoaderPtr->LoadFile(Hash(_file_path));
    if (file && !file->IsExternalFile())
    {
        // Set the cache data
        file_cache.icon_data = file->GetIconData();
        file_cache.file_properties = file->GetFileProperties();
    }

    // Insert the cache entry
    file_cache.file_is_external = file->IsExternalFile();
    auto iter = m_IndexDatas.find(Hash(_file_path));
    if (iter == m_IndexDatas.end())
    {
        return;
    }
    iter->second.file_cache = std::move(file_cache);

    // If this file is an external type, register a watcher for it to enable us calling
    // the file modification callback if for any reason it is modified
    if (file->IsExternalFile())
    {
        // Register a watcher for this file
        m_FileWatcherManager->RequestWatcher(system_filepath, _file_path);
    }
}

void PacketPlainFileIndexer::IndexFileFromPath(Path _file_path)
{
    // Get the system file info
    auto system_filepath = MergeSystemPathWithFilePath(m_PacketPath, _file_path);

    // Check if the file exist
    if (!std::filesystem::exists(system_filepath) || std::filesystem::is_directory(system_filepath))
    {
        return;
    }

    // Setup a new entry
    IndexData index_data = {};
    index_data.file_load_information.file_path = _file_path;
    index_data.file_load_information.file_hash = Hash(_file_path);
    index_data.file_load_information.file_data_size = 0;
    index_data.file_load_information.file_data_size = std::filesystem::file_size(system_filepath);
    index_data.file_extension = system_filepath.extension().string();

    // Add/update the entry
    m_IndexDatas[Hash(_file_path)] = (index_data);

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