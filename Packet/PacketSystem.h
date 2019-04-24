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

    // Register a resource factory
    template <typename ResourceFactoryClass, typename ResourceClass, typename ... Args>
    void RegisterResourceFactory(Args && ... args)
    {
        assert(m_RegisteredFactories.find(ctti::type_id<ResourceClass>().hash()) == m_RegisteredFactories.end());
        m_RegisteredFactories.insert({ ctti::type_id<ResourceClass>().hash(),
                                     std::make_unique<ResourceFactoryClass>(std::forward<Args>(args) ...)});
    }

	// Request an object for the given resource reference and hash
	template <typename ResourceClass>
	bool RequestResource(PacketResourceReference<ResourceClass>& _resourceReference, 
                         Hash _hash,
                         PacketResourceBuildInfo _resourceBuildInfo = PacketResourceBuildInfo())
	{
		assert(m_RegisteredFactories.find(ctti::type_id<ResourceClass>().hash()) != m_RegisteredFactories.end());
		return m_ResourceManager->RequestResource(_resourceReference,
                                                  m_RegisteredFactories[ctti::type_id<ResourceClass>().hash()].get(), 
                                                  _hash, 
                                                  false,
                                                  _resourceBuildInfo);
	}

    // Request an object for the given resource reference and hash
    template <typename ResourceClass>
    void RequestRuntimeResource(PacketResourceReference<ResourceClass>& _resourceReference,
                                PacketResourceBuildInfo _resourceBuildInfo = PacketResourceBuildInfo(),
                                std::vector<uint8_t> _resourceData = {})
    {
        assert(m_RegisteredFactories.find(ctti::type_id<ResourceClass>().hash()) != m_RegisteredFactories.end());
        return m_ResourceManager->RequestRuntimeResource(_resourceReference,
                                                         m_RegisteredFactories[ctti::type_id<ResourceClass>().hash()].get(),
                                                         _resourceBuildInfo, 
                                                         std::move(_resourceData));
    }

	// Request a permanent object for the given reference and resource hash, the object will not be deleted when it reaches 0
	// references, the deletion phase will only occur in conjunction with the storage deletion
	template <typename ResourceClass, typename ResourceInstance>
    bool RequestPermanentResource(PacketResourceReference<ResourceClass>& _resourceReference,
                                  Hash _hash,
                                  PacketResourceBuildInfo _resourceBuildInfo = PacketResourceBuildInfo())
	{
		assert(m_RegisteredFactories.find(ctti::type_id<ResourceClass>().hash()) != m_RegisteredFactories.end());
		return m_ResourceManager->RequestResource(_resourceReference, 
                                                  m_RegisteredFactories[ctti::type_id<ResourceClass>().hash()].get(),
                                                  _hash, 
                                                  true, 
                                                  _resourceBuildInfo);
	}

    // This method will wait until the given resource is ready to be used
    // Optionally you can pass a timeout parameter in milliseconds
    template <typename ResourceClass>
    bool WaitForResource(const PacketResourceReference<ResourceClass>& _resourceReference,
                         long long _timeout = -1) const
    {
        return m_ResourceManager->WaitForResource< ResourceClass>(_resourceReference, _timeout);
    }

    // This method will return a vector of resource external constructor object that must be constructed
    // by the user externally (since the resource requires it)
    std::vector<PacketResourceExternalConstructor> GetResourceExternalConstructors();

    // Return an approximation of the current number of resources since some of them could be enqueued 
    // to be created or released
    uint32_t GetAproximatedResourceAmount() const;

	// Check if a given file exist
	bool FileExist(Hash _fileHash) const;

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
	std::unique_ptr<PacketResourceStorage>  m_ResourceStorage;
	std::unique_ptr<PacketResourceManager>  m_ResourceManager;
	std::unique_ptr<PacketResourceWatcher>  m_ResourceWatcher;

	// All registered resource factories
	std::unordered_map<uint64_t, std::unique_ptr<PacketResourceFactory>> m_RegisteredFactories;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
