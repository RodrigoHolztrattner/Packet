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

PacketSystem::PacketSystem(ThreadIndexRetrieveMethod _threadIndexMethod, uint32_t _totalWorkerThreads)
{
	// Set the initial data
	m_ThreadIndexMethod = _threadIndexMethod;
	m_TotalWorkerThreads = _totalWorkerThreads;
}

PacketSystem::~PacketSystem()
{
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
		m_Logger.get(), 
		m_TotalWorkerThreads,
		m_ThreadIndexMethod);

	return true;
}

bool PacketSystem::FileExist(Hash _fileHash)
{
	return m_FileLoader->FileExist(_fileHash);
}

bool PacketSystem::ConstructPacket()
{
	return m_FileLoader->ConstructPacket();
}

void PacketSystem::Update()
{
	m_ResourceManager->Update();
}