////////////////////////////////////////////////////////////////////////////////
// Filename: PacketSystem.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"

///////////////
// NAMESPACE //
///////////////

// Packet data explorer
PacketDevelopmentNamespaceBegin(Packet)

// Classes we know
class PacketFileIndexer;
class PacketFileImporter;
class PacketFileConverter;
class PacketFileLoader;

typedef std::function<std::optional<PacketFileConverter*>(std::string)> RetrieveFileConverterByExtensionFunc;

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
	bool Initialize(OperationMode _operation_mode, std::filesystem::path _resource_path, std::unique_ptr<PacketLogger>&& _logger = std::make_unique<PacketLogger>());

///////////////
// VARIABLES //
private: //////

    // The resource path and operation mode
    std::filesystem::path m_ResourcePath;
    OperationMode m_OperationMode = OperationMode::Undefined;

	// Our internal objects
    std::unique_ptr<PacketFileIndexer>  m_FileIndexer;
    std::unique_ptr<PacketFileImporter> m_FileImporter;
	std::unique_ptr<PacketFileLoader>   m_FileLoader;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
