////////////////////////////////////////////////////////////////////////////////
// Filename: PacketDataLoader.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketFragment.h"
#include "PacketObjectManager.h"

#include <thread>
#include <mutex>

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
PacketNamespaceBegin(Packet)

////////////////
// FORWARDING //
////////////////

// We know the PacketObject and the PacketFile classes
class PacketObject;

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketDataLoader
////////////////////////////////////////////////////////////////////////////////
class PacketDataLoader
{
public:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketDataLoader(PacketObject* _packetObject);
	~PacketDataLoader();

//////////////////
// MAIN METHODS //
public: //////////

	// Get a file size by it's identifier
	uint32_t GetFileSize(Packet::PacketFragment::FileIdentifier _fileIdentifier);

	// Load a file
	bool GetFileData(Packet::PacketFragment::FileIdentifier _fileIdentifier, unsigned char* _dataLocation);

private:

	// Get the file meta data
	bool GetFileMetadata(PacketFragment::FileIdentifier _fileIdentifier, uint32_t& _fileSize, PacketObjectManager::FileFragmentIdentifier& _fileFragmentIdentifier);

///////////////
// VARIABLES //
private: //////

	// Our packet object reference
	PacketObject* m_PacketObjectReference;

	// Our loading thread and mutex
	std::mutex m_LoadingMutex;
};

// Packet data explorer
PacketNamespaceEnd(Packet)