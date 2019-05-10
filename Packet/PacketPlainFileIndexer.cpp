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
    ScanResourcePath(_resource_path);

    return true;
}

void PacketPlainFileIndexer::ScanResourcePath(std::filesystem::path _resource_path)
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
                InsertResourceIndexData(path);
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

void PacketPlainFileIndexer::InsertResourceIndexData(std::filesystem::path _resource_path)
{





    // m_IndexDatas
}