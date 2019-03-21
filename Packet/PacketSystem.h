////////////////////////////////////////////////////////////////////////////////
// Filename: PacketSystem.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "Resource/PacketResourceManager.h"
#include <ctti//type_id.hpp>
#include <ctti//static_value.hpp>

#include <string>
#include <unordered_map>
#include <assert.h>

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

// Classes we know
class PacketResourceFactory;
class PacketResourceWatcher;
class PacketReferenceManager;
class PacketResourceStorage;
template <typename InstanceClass>
struct PacketResourceInstancePtr;

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
	bool Initialize(OperationMode _operationMode, std::string _packetManifestDirectory, std::unique_ptr<PacketLogger>&& _logger = std::make_unique<PacketLogger>());

	// Pack all files
	bool ConstructPacket();

public:

	// Register a resource factory (takes ownership over the unique_ptr)
	template <typename ResourceClass, typename ResourceFactoryClass>
	void RegisterResourceFactory(std::unique_ptr<ResourceFactoryClass>& _factoryPtr)
	{
		assert(m_RegisteredFactories.find(ctti::type_id<ResourceClass>().hash()) == m_RegisteredFactories.end());
		m_RegisteredFactories.insert({ ctti::type_id<ResourceClass>().hash(), std::move(_factoryPtr )});
	}

	// Register a resource factory (takes ownership over the unique_ptr)
	template <typename ResourceClass, typename ResourceFactoryClass>
	void RegisterResourceFactory(std::unique_ptr<ResourceFactoryClass>&& _factoryPtr)
	{
		assert(m_RegisteredFactories.find(ctti::type_id<ResourceClass>().hash()) == m_RegisteredFactories.end());
		m_RegisteredFactories.insert({ ctti::type_id<ResourceClass>().hash(), std::move(_factoryPtr) });
	}

	// Request an object for the given instance and resource hash
	template <typename ResourceClass, typename ResourceInstance>
	bool RequestResource(PacketResourceInstancePtr<ResourceInstance>& _instancePtr, Hash _hash, PacketResourceBuildInfo _resourceBuildInfo = PacketResourceBuildInfo())
	{
		assert(m_RegisteredFactories.find(ctti::type_id<ResourceClass>().hash()) != m_RegisteredFactories.end());
		return m_ResourceManager->RequestResource(_instancePtr, _hash, m_RegisteredFactories[ctti::type_id<ResourceClass>().hash()].get(), false, _resourceBuildInfo);
	}

	// Request a permanent object for the given instance and resource hash, the object will not be deleted when it reaches 0
	// references, the deletion phase will only occur in conjunction with the storage deletion
	template <typename ResourceClass, typename ResourceInstance>
	bool RequestPersistentResource(PacketResourceInstancePtr<ResourceInstance>& _instancePtr, Hash _hash, PacketResourceBuildInfo _resourceBuildInfo = PacketResourceBuildInfo())
	{
		assert(m_RegisteredFactories.find(ctti::type_id<ResourceClass>().hash()) != m_RegisteredFactories.end());
		return m_ResourceManager->RequestResource(_instancePtr, _hash, m_RegisteredFactories[ctti::type_id<ResourceClass>().hash()].get(), true, _resourceBuildInfo);
	}

	// Check if a given file exist
	bool FileExist(Hash _fileHash) const;

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

	// The logger object
	std::unique_ptr<PacketLogger> m_Logger;

	// The reference manager, the resource storage, our current resource manager and the resource watcher
	std::unique_ptr<PacketReferenceManager> m_ReferenceManager;
	std::unique_ptr<PacketResourceStorage> m_ResourceStorage;
	std::unique_ptr<PacketResourceManager> m_ResourceManager;
	std::unique_ptr<PacketResourceWatcher> m_ResourceWatcher;

	// All registered resource factories
	std::unordered_map<uint64_t, std::unique_ptr<PacketResourceFactory>> m_RegisteredFactories;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
