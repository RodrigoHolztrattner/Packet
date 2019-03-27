////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceWatcher.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "..\PacketConfig.h"
#include "PacketResource.h"
#include "..\ThirdParty\FileWatcher\FileWatcher.h"

#include <map>
#include <functional>

///////////////
// NAMESPACE //
///////////////

/////////////
// DEFINES //
/////////////

////////////
// GLOBAL //
////////////

///////////////
// NAMESPACE //
///////////////

// Packet
PacketDevelopmentNamespaceBegin(Packet)

//////////////
// TYPEDEFS //
//////////////

////////////////
// FORWARDING //
////////////////

// Classes we know
class PacketResourceLoader;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketResourceWatcher
////////////////////////////////////////////////////////////////////////////////
class PacketResourceWatcher
{
public:

	// The watch listener object type
	struct ResourceWatchListener : public FWPacket::FileWatchListener
	{
		// Create passing a reference to the resource watcher
		ResourceWatchListener(PacketResourceWatcher* _packetResourceWatcher) : m_ResourceWatcherPtr(_packetResourceWatcher) {}

		// Forward the file action to the resource watcher object
		void handleFileAction(FWPacket::WatchID _watchid, const FWPacket::String& _dir, const FWPacket::String& _filename, FWPacket::Action _action) override
		{
			m_ResourceWatcherPtr->HandleFileAction(_watchid, _dir, _filename, _action);
		}

	private:

		// A pointer to the resource watcher object
		PacketResourceWatcher* m_ResourceWatcherPtr;
	};

	// A watched directory listener type
	struct WatchedDirectory
	{
		// The watch ID for this directory
		FWPacket::WatchID watchID;

		// All watched resources inside this directory
		std::map<HashPrimitive, PacketResource*> watchedResources;
	};

	// Friend classes/structs
	friend ResourceWatchListener;

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketResourceWatcher(OperationMode _operationMode);
	~PacketResourceWatcher();

//////////////////
// MAIN METHODS //
public: //////////

	// This method will add a watch to a resource (the resource don't need to be fully loaded but it must has it hash 
	// object updated
	bool WatchResource(PacketResource* _resource);

	// This method will return a watch from the given resource (same conditions as the WatchResource() above)
	void RemoveWatch(PacketResource* _resource);

	// This method will update a watched resource so it will pointos to the given resource
	void UpdateWatchedResource(PacketResource* _resource);

	// Register the on data changed callback method
	void RegisterOnResourceDataChangedMethod(std::function<void(PacketResource*)> _method);

	// Update the file watcher system
	void Update();

protected:

	// The handle file action method
	void HandleFileAction(FWPacket::WatchID _watchid, const FWPacket::String& _dir, const FWPacket::String& _filename, FWPacket::Action _action);

///////////////
// VARIABLES //
private: //////

	// If this resource watcher is enabled (if the operation mode allows us to use the watch functionality)
	bool m_IsEnabled;

	// The resource file watcher object
	FWPacket::FileWatcher m_ResourceFileWatcher;

	// The resource watcher listener object
	ResourceWatchListener m_ResourceWatcherListener;

	// All watched directories
	std::map<HashPrimitive, WatchedDirectory> m_WatchedDirectories;

	// The OnResourceDataChanged() callback
	std::function<void(PacketResource*)> m_ResourceDataChangedCallback;
};

// Packet
PacketDevelopmentNamespaceEnd(Packet)
