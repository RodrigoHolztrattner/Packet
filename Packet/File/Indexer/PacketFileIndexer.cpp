////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileIndexer.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileIndexer.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketFileIndexer::PacketFileIndexer(std::filesystem::path _packet_path) :
    m_PacketPath(_packet_path)
{
	// Set the initial data
	// ...
}

PacketFileIndexer::~PacketFileIndexer()
{
}

void PacketFileIndexer::SetAuxiliarObjects(const PacketFileLoader* _file_loader)
{
    m_FileLoaderPtr = _file_loader;
}

void PacketFileIndexer::RegisterFileModificationCallback(FileModificationCallback _callback)
{
    m_FileModificationCallbacks.push_back(_callback);
}

void PacketFileIndexer::UnregisterFileWatcher(PacketFileWatcher* _file_watcher_ptr)
{
    std::lock_guard lock(m_Mutex);

    auto iter = m_FileWatchers.find(_file_watcher_ptr->file_hash());
    if (iter == m_FileWatchers.end())
    {
        return;
    }

    iter->second.erase(_file_watcher_ptr);
}

void PacketFileIndexer::PropagateFileModificationToWatchers(Path _file_path) const
{
    // Call all registered callbacks since this file was modified/added
    for (auto& callback : m_FileModificationCallbacks)
    {
        callback(_file_path);
    }

    // If this file have a file watcher attached
    {
        auto iter = m_FileWatchers.find(Hash(_file_path));
        if (iter != m_FileWatchers.end())
        {
            // Call each watcher callback
            for (auto& [watcher_ptr, watcher] : iter->second)
            {
                watcher->OnFileChange();
            }
        }
    }
}