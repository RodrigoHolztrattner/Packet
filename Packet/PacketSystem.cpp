////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketSystem.h"
#include "File/PacketFileManager.h"
#include "Resource/PacketResourceManager.h"
#include "Resource/Storage/PacketResourceStorage.h"

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
    m_ResourceStorage.reset();
    m_FileManager.reset();
    m_Logger.reset();
}

bool PacketSystem::Initialize(OperationMode _operation_mode, std::filesystem::path _resource_path, std::unique_ptr<PacketLogger>&& _logger)
{
    // Set the resource path and the operation mode
    m_OperationMode = _operation_mode;
    m_ResourcePath = _resource_path;
    m_Logger = std::move(_logger);

    // Create and initialize the file manager
    m_FileManager = std::make_unique<PacketFileManager>(_operation_mode, _resource_path);
    if (!m_FileManager->Initialize())
    {
        return false;
    }

    // Create the resource storage
    m_ResourceStorage = std::make_unique<PacketResourceStorage>();

    // Create the resource manager
    m_ResourceManager = std::make_unique<PacketResourceManager>(
        m_OperationMode,
        *m_ResourceStorage,
        m_FileManager->GetFileLoader(),
        m_FileManager->GetFileIndexer(),
        m_Logger.get());

	return true;
}

std::filesystem::path PacketSystem::GetResourcePath() const
{
    return m_ResourcePath;
}

PacketFileManager& PacketSystem::GetFileManager()
{
    return *m_FileManager;
}

PacketResourceManager& PacketSystem::GetResourceManager()
{
    return *m_ResourceManager;
}