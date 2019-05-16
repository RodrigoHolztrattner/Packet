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
    return m_IndexDatas.find(_file_hash) != m_IndexDatas.end();
}

std::optional<PacketFileIndexer::FileLoadInformation> PacketPlainFileIndexer::RetrieveFileLoadInformation(HashPrimitive _file_hash) const
{
    auto iter = m_IndexDatas.find(_file_hash);
    if (iter != m_IndexDatas.end())
    {
        return m_IndexDatas.find(_file_hash)->second.file_load_information;
    }

    return std::nullopt;
}

std::optional<std::string> PacketPlainFileIndexer::GetFileExtension(HashPrimitive _file_hash) const
{
    auto iter = m_IndexDatas.find(_file_hash);
    if (iter != m_IndexDatas.end())
    {
        return m_IndexDatas.find(_file_hash)->second.file_extension;
    }

    return std::nullopt;
}

std::optional<PacketFileHeader> PacketPlainFileIndexer::GetFileHeader(HashPrimitive _file_hash) const
{
    auto iter = m_IndexDatas.find(_file_hash);
    if (iter != m_IndexDatas.end())
    {
        return m_IndexDatas.find(_file_hash)->second.file_header;
    }

    return std::nullopt;
}

std::vector<uint8_t> PacketPlainFileIndexer::GetFileIconData(HashPrimitive _file_hash) const
{
    auto iter = m_IndexDatas.find(_file_hash);
    if (iter != m_IndexDatas.end())
    {
        return m_IndexDatas.find(_file_hash)->second.icon_data;
    }

    return std::vector<uint8_t>();
}

nlohmann::json PacketPlainFileIndexer::GetFileProperties(HashPrimitive _file_hash) const
{
    auto iter = m_IndexDatas.find(_file_hash);
    if (iter != m_IndexDatas.end())
    {
        return m_IndexDatas.find(_file_hash)->second.file_properties;

    }

    return nlohmann::json();
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
            // Folder (ignore the internal folder)
            else if (path.filename() != InternalFolderName)
            {
                // Enter this folder and continue searching
                ScanResourcePathRecursive(path);
            }
        }
    };

    // Call the scan method for the base resource path
    ScanResourcePathRecursive(_resource_path);
}

void PacketPlainFileIndexer::InsertFileIndexData(Path _file_path)
{
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