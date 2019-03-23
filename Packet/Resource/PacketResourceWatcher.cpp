////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceWatcher.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResourceWatcher.h"
#include "PacketResourceFactory.h"
#include <cassert>
#include <filesystem>

// Using namespace Peasant
PacketUsingDevelopmentNamespace(Packet)

PacketResourceWatcher::PacketResourceWatcher(OperationMode _operationMode) : m_ResourceWatcherListener(this)
{
	// Check the operation mode
	if (_operationMode == OperationMode::Edit)
	{
		// On edit mode we are enabled
		m_IsEnabled = true;
	}
	// Different from edit mode
	else
	{
		// On all other modes we are disabled
		m_IsEnabled = false;
	}
}

PacketResourceWatcher::~PacketResourceWatcher()
{
}

void PacketResourceWatcher::HandleFileAction(FWPacket::WatchID _watchid, const FWPacket::String& _dir, const FWPacket::String& _filename, FWPacket::Action _action)
{
	// Check if we have a watch on this directory
	auto iter = m_WatchedDirectories.find(Hash(_dir).GetHashValue());
	if (iter == m_WatchedDirectories.end())
	{
		return;
	}

	// Check if the resource exist
	auto resourceIter = iter->second.watchedResources.find(Hash(_filename).GetHashValue());
	if (resourceIter == iter->second.watchedResources.end())
	{
		return;
	}

	// Get the resource object
	PacketResource* resource = resourceIter->second;

	// Check what action happened
	switch (_action)
	{
	case FWPacket::Actions::Add:
		break;
	case FWPacket::Actions::Delete:
		break;
	case FWPacket::Actions::Modified:

		// Call the callback method
		m_ResourceDataChangedCallback(resource);
		break;

	default:
		assert(false);
	}
}

bool PacketResourceWatcher::WatchResource(PacketResource* _resource)
{
    // If this is a runtime resource, don't do anything
    if (_resource->IsRuntime())
    {
        return true;
    }

	// If we are enabled, continue
	if (!m_IsEnabled)
	{
		return false;
	}

	// Convert the resource hash path to the filesystem type
	auto path = std::filesystem::path(_resource->GetHash().GetPath().String());

	// Hash the path string
	Hash pathHash = Hash(path.parent_path().string());

	// Check if we already have a watch on this directory
	auto iter = m_WatchedDirectories.find(pathHash);
	if (iter == m_WatchedDirectories.end())
	{
		// Prepare a new watcher for this directory
		WatchedDirectory watchedDir = {};
		watchedDir.watchID = m_ResourceFileWatcher.addWatch(path.parent_path().string(), &m_ResourceWatcherListener);

		// Insert it
		m_WatchedDirectories.insert({ pathHash, watchedDir });

		// Update the iterator
		iter = m_WatchedDirectories.find(pathHash);
	}

	// Get the filename
	auto filename = path.filename();

	// Insert the resource into the watched resources
	iter->second.watchedResources.insert({ Hash(filename.string()).GetHashValue(), _resource });
	
	return true;
}

void PacketResourceWatcher::RemoveWatch(PacketResource* _resource)
{
    // If this is a runtime resource, don't do anything
    if (_resource->IsRuntime())
    {
        return;
    }

	// If we are enabled, continue
	if (!m_IsEnabled)
	{
		return;
	}

	// Convert the resource hash path to the filesystem type
	auto path = std::filesystem::path(_resource->GetHash().GetPath().String());

	// Hash the path string
	Hash pathHash = Hash(path.parent_path().string());

	// Get the filename
	auto filename = path.filename();

	// Check if we have a watch on this directory
	auto iter = m_WatchedDirectories.find(pathHash);
	if (iter == m_WatchedDirectories.end())
	{
		return;
	}

	// Check if we have this file and the ptrs match (if they don't match is because this resource 
	// was replaced by another one and we shouldn't remove its watch)
	auto resourceIter = iter->second.watchedResources.find(Hash(filename.string()).GetHashValue());
	if (resourceIter == iter->second.watchedResources.end() || resourceIter->second != _resource)
	{
		return;
	}

	// Remove the file
	iter->second.watchedResources.erase(resourceIter);

	// Check if we have at last one resource being watched
	if (iter->second.watchedResources.size() == 0)
	{
		// Stop watching this directory
		m_ResourceFileWatcher.removeWatch(iter->second.watchID);

		// Remove the directory from the map
		m_WatchedDirectories.erase(iter);
	}
}

void PacketResourceWatcher::UpdateWatchedResource(PacketResource* _resource)
{
    // If this is a runtime resource, don't do anything
    if (_resource->IsRuntime())
    {
        return;
    }

	// If we are enabled, continue
	if (!m_IsEnabled)
	{
		return;
	}

	// Convert the resource hash path to the filesystem type
	auto path = std::filesystem::path(_resource->GetHash().GetPath().String());

	// Hash the path string
	Hash pathHash = Hash(path.parent_path().string());

	// Get the filename
	auto filename = path.filename();

	// Check if we have a watch on this directory
	auto iter = m_WatchedDirectories.find(pathHash);
	if (iter == m_WatchedDirectories.end())
	{
		return;
	}

	// Get the file iterator
	auto fileIter = iter->second.watchedResources.find(Hash(filename.string()).GetHashValue());
	if (fileIter == iter->second.watchedResources.end())
	{
		return;
	}

	// Replace the file
	fileIter->second = _resource;
}

void PacketResourceWatcher::Update()
{
	// If we are enabled, continue
	if (!m_IsEnabled)
	{
		return;
	}

	// Call the update method for the file watcher object, so it can detect file changes
	m_ResourceFileWatcher.update();
}

void PacketResourceWatcher::RegisterOnResourceDataChangedMethod(std::function<void(PacketResource*)> _method)
{
	m_ResourceDataChangedCallback = _method;
}
