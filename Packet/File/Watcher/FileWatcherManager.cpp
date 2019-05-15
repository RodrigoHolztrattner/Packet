////////////////////////////////////////////////////////////////////////////////
// Filename: FileWatcherManager.cpp
////////////////////////////////////////////////////////////////////////////////
#include "FileWatcherManager.h"
#include <cassert>
#include <filesystem>

// Using namespace Peasant
PacketUsingDevelopmentNamespace(Packet)

FileWatcherManager::FileWatcherManager(bool _use_dedicated_thread) :
    m_FileWatcherListener(this)
{
	// Create the thread that will periodically check for file updates
    if (_use_dedicated_thread)
    {
        m_FileUpdateThread = std::thread(
            [&]()
            {
                while (!m_ExitFileUpdateThread)
                {
                    // Call the update method
                    Update();

                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            });
    }
}

FileWatcherManager::~FileWatcherManager()
{
    // Join, if possible, the file update thread
    m_ExitFileUpdateThread = true;
    if (m_FileUpdateThread.joinable())
    {
        m_FileUpdateThread.join();
    }
}

void FileWatcherManager::HandleFileAction(FWPacket::WatchID, const FWPacket::String& _dir, const FWPacket::String& _filename, FWPacket::Action _action)
{
	// Check if we have a watch on this directory
	auto iter = m_WatchedDirectories.find(Hash(_dir).GetHashValue());
	if (iter == m_WatchedDirectories.end())
	{
		return;
	}

	// Check if the resource exist
	auto resourceIter = iter->second.watchedFiles.find(Hash(_filename).GetHashValue());
	if (resourceIter == iter->second.watchedFiles.end())
	{
		return;
	}

	// Get the resource object
    Path file_path = resourceIter->second;

	// Check what action happened
	switch (_action)
	{
	case FWPacket::Actions::Add:
		break;
	case FWPacket::Actions::Delete:
		break;
	case FWPacket::Actions::Modified:

		// Call the callback method
		m_FileDataChangedCallback(file_path);
		break;

	default:
		assert(false);
	}
}

bool FileWatcherManager::WatchFilePath(std::filesystem::path _system_path, Path _file_path)
{
	// Hash the path string
	Hash pathHash = Hash(_system_path.string());

	// Check if we already have a watch on this directory
	auto iter = m_WatchedDirectories.find(pathHash);
	if (iter == m_WatchedDirectories.end())
	{
		// Prepare a new watcher for this directory
		WatchedDirectory watchedDir = {};
		watchedDir.watchID = m_FileWatcher.addWatch(_system_path.parent_path().string(), &m_FileWatcherListener);

		// Insert it
		m_WatchedDirectories.insert({ pathHash, watchedDir });

		// Update the iterator
		iter = m_WatchedDirectories.find(pathHash);
	}

	// Get the filename
	auto filename = _system_path.filename();

	// Insert the resource into the watched resources
	iter->second.watchedFiles.insert({ Hash(filename.string()).GetHashValue(), _file_path });
	
	return true;
}

void FileWatcherManager::RemoveWatch(std::filesystem::path _system_path)
{
	// Hash the path string
	Hash pathHash = Hash(_system_path.string());

	// Get the filename
	auto filename = _system_path.filename();

	// Check if we have a watch on this directory
	auto iter = m_WatchedDirectories.find(pathHash);
	if (iter == m_WatchedDirectories.end())
	{
		return;
	}

	// Check if we have this file and the ptrs match
	auto resourceIter = iter->second.watchedFiles.find(Hash(filename.string()).GetHashValue());
	if (resourceIter == iter->second.watchedFiles.end())
	{
		return;
	}

	// Remove the file
	iter->second.watchedFiles.erase(resourceIter);

	// Check if we have at last one resource being watched
	if (iter->second.watchedFiles.size() == 0)
	{
		// Stop watching this directory
		m_FileWatcher.removeWatch(iter->second.watchID);

		// Remove the directory from the map
		m_WatchedDirectories.erase(iter);
	}
}

void FileWatcherManager::Update()
{
	// Call the update method for the file watcher object, so it can detect file changes
	m_FileWatcher.update();
}

void FileWatcherManager::RegisterOnFileDataChangedMethod(std::function<void(Path _file_path)> _method)
{
	m_FileDataChangedCallback = _method;
}
