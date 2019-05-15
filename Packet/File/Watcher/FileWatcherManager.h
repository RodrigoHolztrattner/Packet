////////////////////////////////////////////////////////////////////////////////
// Filename: FileWatcherManager.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "../../PacketConfig.h"
#include "FileWatcher.h"
#include <map>
#include <functional>
#include <thread>

///////////////
// NAMESPACE //
///////////////

// Packet
PacketDevelopmentNamespaceBegin(Packet)

////////////////////////////////////////////////////////////////////////////////
// Class name: FileWatcherManager
////////////////////////////////////////////////////////////////////////////////
class FileWatcherManager
{
public:

	// The watch listener object type
	struct FileWatchListener : public FWPacket::FileWatchListener
	{
		// Create passing a reference to the resource watcher
		FileWatchListener(FileWatcherManager* _FileWatcherManager) : m_FileWatcherPtr(_FileWatcherManager) {}

		// Forward the file action to the resource watcher object
		void handleFileAction(FWPacket::WatchID _watchid, const FWPacket::String& _dir, const FWPacket::String& _filename, FWPacket::Action _action) override
		{
			m_FileWatcherPtr->HandleFileAction(_watchid, _dir, _filename, _action);
		}

	private:

		// A pointer to the resource watcher object
		FileWatcherManager* m_FileWatcherPtr;
	};

	// A watched directory listener type
	struct WatchedDirectory
	{
		// The watch ID for this directory
		FWPacket::WatchID watchID;

		// All watched file paths inside this directory
		std::map<HashPrimitive, Path> watchedFiles;
	};

	// Friend classes/structs
	friend FileWatchListener;

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	FileWatcherManager(bool _use_dedicated_thread);
	~FileWatcherManager();

//////////////////
// MAIN METHODS //
public: //////////

	// This method will add a watch to a resource (the resource don't need to be fully loaded but it must has it hash 
	// object updated
	bool WatchFilePath(std::filesystem::path _system_path, Path _file_path);

	// This method will return a watch from the given resource (same conditions as the WatchFile() above)
	void RemoveWatch(std::filesystem::path _system_path);

	// Register the on data changed callback method
	void RegisterOnFileDataChangedMethod(std::function<void(Path _file_path)> _method);

	// Update the file watcher system
	void Update();

protected:

	// The handle file action method
	void HandleFileAction(FWPacket::WatchID _watchid, const FWPacket::String& _dir, const FWPacket::String& _filename, FWPacket::Action _action);

///////////////
// VARIABLES //
private: //////

	// The thread that will periodically check for changes and its exit flag
    std::thread m_FileUpdateThread;
    bool m_ExitFileUpdateThread = false;

	// The file watcher object
	FWPacket::FileWatcher m_FileWatcher;

	// The file watcher listener object
	FileWatchListener m_FileWatcherListener;

	// All watched directories
	std::map<HashPrimitive, WatchedDirectory> m_WatchedDirectories;

	// The OnFileDataChanged() callback
	std::function<void(Path _file_path)> m_FileDataChangedCallback;
};

// Packet
PacketDevelopmentNamespaceEnd(Packet)
