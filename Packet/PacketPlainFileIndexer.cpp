////////////////////////////////////////////////////////////////////////////////
// Filename: PacketPlainFileIndexer.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketPlainFileIndexer.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketPlainFileIndexer::PacketPlainFileIndexer()
{
	// Set the initial data
	// ...
}

PacketPlainFileIndexer::~PacketPlainFileIndexer()
{
}

bool PacketPlainFileIndexer::Initialize(std::filesystem::path _resource_path)
{
    // Scan the resource path
    ScanSystemForFiles(_resource_path);

    return true;
}

bool PacketPlainFileIndexer::IsFileIndexed(HashPrimitive _file_hash) const
{
    return m_IndexDatas.find(_file_hash) != m_IndexDatas.end();
}

std::optional<PacketFileIndexer::FileLoadInformation> PacketPlainFileIndexer::RetrieveFileLoadInformation(HashPrimitive _file_hash) const
{
    // TODO:
    return std::nullopt;
}

const PacketFileHeader& PacketPlainFileIndexer::GetFileHeader(HashPrimitive _file_hash) const
{
    if (m_IndexDatas.find(_file_hash) == m_IndexDatas.end())
    {
        throw "Invalid file hash";
    }

    return m_IndexDatas.find(_file_hash)->second.file_header;
}

const std::vector<uint8_t>& PacketPlainFileIndexer::GetFileIconData(HashPrimitive _file_hash) const
{
    if (m_IndexDatas.find(_file_hash) == m_IndexDatas.end())
    {
        throw "Invalid file hash";
    }

    return m_IndexDatas.find(_file_hash)->second.icon_data;
}

void PacketPlainFileIndexer::ScanSystemForFiles(std::filesystem::path _resource_path)
{
    // This method will recursively construct the packet tree
    std::function<void(const std::filesystem::path&)> ScanResourcePathRecursive = [&](const std::filesystem::path& _current_path)
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
            else if(path.filename() != InternalFolderName)
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





    // m_IndexDatas
}

void PacketPlainFileIndexer::RemoveFileIndexData(Path _file_path)
{

}