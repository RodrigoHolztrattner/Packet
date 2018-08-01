////////////////////////////////////////////////////////////////////////////////
// Filename: PacketSystem.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketReferenceManager.h"
#include "Resource/PacketResourceManager.h"
#include "Resource/PacketResourceWatcher.h"

#include <string>

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

// Packet data explorer
PacketDevelopmentNamespaceBegin(Packet)

////////////////
// FORWARDING //
////////////////

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketSystem
////////////////////////////////////////////////////////////////////////////////
class PacketSystem
{
public:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketSystem();
	~PacketSystem();

//////////////////
// MAIN METHODS //
public: //////////

	// Initialize this object
	bool Initialize(std::string _packetManifestDirectory, OperationMode _operationMode);

	// Pack all files
	bool ConstructPacket();

	// Return a ptr to the reference manager
	PacketReferenceManager* GetReferenceManager();

public:

	// Request an object for the given instance and resource hash
	template <typename ResourceInstance, typename ResourceFactory>
	bool RequestObject(PacketResourceInstancePtr<ResourceInstance>& _instancePtr, Hash _hash, ResourceFactory* _factoryPtr, bool _allowAsynchronousConstruct = true)
	{
		return m_ResourceManager->RequestObject(_instancePtr, _hash, _factoryPtr, false, _allowAsynchronousConstruct);
	}

	// Request a permanent object for the given instance and resource hash, the object will not be deleted when it reaches 0
	// references, the deletion phase will only occur in conjunction with the storage deletion
	template <typename ResourceInstance, typename ResourceFactory>
	bool RequestPersistentObject(PacketResourceInstancePtr<ResourceInstance>& _instancePtr, Hash _hash, ResourceFactory* _factoryPtr, bool _allowAsynchronousConstruct = true)
	{
		return m_ResourceManager->RequestObject(_instancePtr, _hash, _factoryPtr, true, _allowAsynchronousConstruct);
	}

	// Check if a given file exist
	bool FileExist(Hash _fileHash);

	// Query the loaded objects map, returning a reference to it
//	std::map<Hash, PacketResource*>& QueryLoadedObjects();

	// The update method, process all requests
	void Update();

///////////////
// VARIABLES //
private: //////

	// The operation mode
	OperationMode m_OperationMode;

	// The packet folder path
	std::string m_PacketFolderPath;

private:

	// Our packet file loader
	std::unique_ptr<PacketFileLoader> m_FileLoader;

	// The reference manager
	PacketReferenceManager m_ReferenceManager;

	// The resource storage, our current resource manager and the resource watcher
	std::unique_ptr<PacketResourceStorage> m_ResourceStorage;
	std::unique_ptr<PacketResourceManager> m_ResourceManager;
	std::unique_ptr<PacketResourceWatcher> m_ResourceWatcher;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
