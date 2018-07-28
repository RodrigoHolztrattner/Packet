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

	// Initialize the reference manager
	m_ReferenceManager.Initialize(_packetManifestDirectory);

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

	return &m_ReferenceManager;
}