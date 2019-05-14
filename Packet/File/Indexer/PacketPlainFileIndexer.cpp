////////////////////////////////////////////////////////////////////////////////
// Filename: PacketPlainFileIndexer.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketPlainFileIndexer.h"
#include "../Loader/PacketFileLoader.h"

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
}

void PacketPlainFileIndexer::RemoveFileIndexData(Path _file_path)
{
    // Find an entry with this path
    auto iter = m_IndexDatas.find(Hash(_file_path));
    if (iter != m_IndexDatas.end())
    {
        m_IndexDatas.erase(iter);
    }
}