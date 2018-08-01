////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketSystem.h"
#include <filesystem>
#include "PacketEditModeFileLoader.h"
#include "PacketCondensedModeFileLoader.h"

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
}

bool PacketSystem::Initialize(std::string _packetManifestDirectory, OperationMode _operationMode)
{
	// Set the data
	m_PacketFolderPath = _packetManifestDirectory;
	m_OperationMode = _operationMode;

	// Create and initialize the reference manager
	m_ReferenceManager = std::make_unique<PacketReferenceManager>();
	m_ReferenceManager->Initialize(_packetManifestDirectory);

	// Create our file loader using the given operation mode
	if (_operationMode == OperationMode::Edit)
	{
		// Create the file loader
		m_FileLoader = std::make_unique<PacketEditModeFileLoader>(_packetManifestDirectory);
	}
	// Condensed
	else
	{
		// Create the file loader
		m_FileLoader = std::make_unique<PacketCondensedModeFileLoader>(_packetManifestDirectory);
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
		1, 
		nullptr);

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

PacketReferenceManager* PacketSystem::GetReferenceManager()
{
	if (m_OperationMode != OperationMode::Edit)
	{
		return nullptr;
	}

	return m_ReferenceManager.get();
}

void PacketSystem::Update()
{
	m_ResourceManager->Update();
}