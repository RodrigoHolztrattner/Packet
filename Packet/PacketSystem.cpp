////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketSystem.h"

#include "PacketFileIndexer.h"
#include "PacketPlainFileIndexer.h"

#include "PacketFileLoader.h"
#include "PacketPlainFileLoader.h"

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

    // Depending on our operation mode, create the appropriated internal objects
    if (m_OperationMode == OperationMode::Plain)
    {
        m_FileIndexer = std::make_unique<PacketPlainFileIndexer>();
        m_FileLoader = std::make_unique<PacketPlainFileLoader>(*m_FileIndexer);
        m_FileImporter = std::make_unique<PacketFileImporter>();
    }
    // Condensed
    else
    {
        // m_FileIndexer = std::make_unique<PacketCondensedFileIndexer>();
    }

    // Initialize the file indexer
    if (!m_FileIndexer->Initialize(_resource_path))
    {
        return false;
    }

	return true;
}