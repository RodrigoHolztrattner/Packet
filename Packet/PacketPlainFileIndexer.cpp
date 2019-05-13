////////////////////////////////////////////////////////////////////////////////
// Filename: PacketPlainFileIndexer.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketPlainFileIndexer.h"
#include "PacketFileLoader.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketPlainFileIndexer::PacketPlainFileIndexer(std::filesystem::path _packet_path) : 
    PacketFileIndexer(_packet_path)
{
	// Set the initial data
	// ...
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
    // Create a new root node
    m_RootNode = std::make_unique<DirectoryNode>();
    m_RootNode->directory_name = _resource_path.filename().string();
    m_RootNode->directory_path = ConvertSystemPathIntoInternalPath(_resource_path, _resource_path);
    m_RootNode->directory_internal_path = _resource_path;

    // This method will recursively construct the packet tree
    std::function<void(DirectoryNode&)> CreateDirectoryTreeRecursive = [&](DirectoryNode& _current_node)
    {
        for (auto& p : std::filesystem::directory_iterator(_current_node.directory_internal_path))
        {
            if (p.is_directory())
            {
                _current_node.children_folders.push_back(std::make_unique<DirectoryNode>());
                DirectoryNode& child_node = *_current_node.children_folders.back();

                child_node.directory_name = p.path().filename().string();
                child_node.directory_path = ConvertSystemPathIntoInternalPath(_resource_path, p);
                child_node.directory_internal_path = p.path();

                CreateDirectoryTreeRecursive(child_node);
            }
            else
            {
                auto file_path = ConvertSystemPathIntoInternalPath(_resource_path, p);
                _current_node.children_files.push_back(file_path);

                InsertFileIndexData(file_path);
            }
        }
    };

    // Call the scan method for the base resource path
    CreateDirectoryTreeRecursive(*m_RootNode);
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

    // Load this file header, icon data and properties
    // TODO: This can be optimized (we are doing 2 file reads here)
    auto file_index_data = m_FileLoaderPtr->LoadFileDataPart(Hash(_file_path), FilePart::IconData);
    auto file_property_data = m_FileLoaderPtr->LoadFileDataPart(Hash(_file_path), FilePart::PropertiesData);
    if (file_index_data && file_property_data)
    {
        // Set the header, icon data and properties
        auto& [header, icon_data] = file_index_data.value();
        auto& [_, property_data] = file_property_data.value();
        new_index_entry.file_header = header;
        new_index_entry.icon_data = std::move(icon_data);
        new_index_entry.file_properties = property_data;
    }
 
    // Set the file extension and load data
    new_index_entry.file_extension = system_filepath.extension().string();
    new_index_entry.file_load_information.file_path = _file_path;
    new_index_entry.file_load_information.file_hash = Hash(_file_path);
    new_index_entry.file_load_information.file_data_position = 0;
    new_index_entry.file_load_information.file_data_size = std::filesystem::file_size(system_filepath);

    // Add the entry
    m_IndexDatas.insert({ Hash(_file_path), std::move(new_index_entry) });

    // Rebuild the filesystem view
    BuildFilesystemView(m_PacketPath);
}

void PacketPlainFileIndexer::RemoveFileIndexData(Path _file_path)
{
    // Find an entry with this path
    auto iter = m_IndexDatas.find(Hash(_file_path));
    if (iter != m_IndexDatas.end())
    {
        m_IndexDatas.erase(iter);
    }

    // Rebuild the filesystem view
    BuildFilesystemView(m_PacketPath);
}