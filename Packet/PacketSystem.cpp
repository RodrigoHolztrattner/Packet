////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketSystem.h"
#include "PacketFileManager.h"

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

bool PacketSystem::Initialize(OperationMode _operation_mode, std::filesystem::path _resource_path, std::unique_ptr<PacketLogger>&& _logger)
{
    // Set the resource path and the operation mode
    assert(_operation_mode == OperationMode::Undefined);
    m_OperationMode = _operation_mode;
    m_ResourcePath = _resource_path;

    // Create and initialize the file manager
    m_FileManager = std::make_unique<PacketFileManager>(_operation_mode, _resource_path);
    if (!m_FileManager->Initialize())
    {
        return false;
    }

	return true;
}