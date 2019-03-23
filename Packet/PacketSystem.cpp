////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketSystem.h"
#include <filesystem>
#include "PacketEditModeFileLoader.h"
#include "PacketCondensedModeFileLoader.h"
#include "PacketReferenceManager.h"
#include "Resource/PacketResourceFactory.h"
#include "Resource/PacketResourceWatcher.h"
#include "Resource/PacketResourceManager.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketSystem::PacketSystem()
{
	// Set the initial data
	// ...
}

PacketSystem::~PacketSystem()
{
    // We need to respect an order of destruction here to prevent accessing deleted objects
    m_ResourceManager.reset();
    m_ResourceWatcher.reset();
    m_ResourceStorage.reset();
    m_ReferenceManager.reset();
    m_RegisteredFactories.clear();
    m_FileLoader.reset();
    m_Logger.reset();
}

bool PacketSystem::Initialize(OperationMode _operationMode, std::string _packetManifestDirectory, std::unique_ptr<PacketLogger>&& _logger)
{
	// Set the data
	m_PacketFolderPath = _packetManifestDirectory;
	m_Logger = std::move(_logger);
	m_OperationMode = std::move(_operationMode);

	// Create and initialize the reference manager
	m_ReferenceManager = std::make_unique<PacketReferenceManager>();
	m_ReferenceManager->Initialize(_packetManifestDirectory, m_Logger.get());

	// Create our file loader using the given operation mode
	if (_operationMode == OperationMode::Edit)
	{
		// Create the file loader
		m_FileLoader = std::make_unique<PacketEditModeFileLoader>(_packetManifestDirectory, m_Logger.get());
	}
	// Condensed
	else
	{
		// Create the file loader
		m_FileLoader = std::make_unique<PacketCondensedModeFileLoader>(_packetManifestDirectory, m_Logger.get());
	}

	// Create the resource storage
	m_ResourceStorage = std::make_unique<PacketResourceStorage>();

	// Create the resource watcher
	m_ResourceWatcher = std::make_unique<PacketResourceWatcher>(_operationMode);

	// Create the resource manager
	m_ResourceManager = std::make_unique<PacketResourceManager>(_operationMode, 
		m_ResourceStorage.get(), 
		m_FileLoader.get(), 
		m_ReferenceManager.get(), 
		m_ResourceWatcher.get(), 
		m_Logger.get());

	return true;
}

bool PacketSystem::FileExist(Hash _fileHash) const
{
	return m_FileLoader->FileExist(_fileHash);
}

bool PacketSystem::ConstructPacket()
{
	return m_FileLoader->ConstructPacket();
}