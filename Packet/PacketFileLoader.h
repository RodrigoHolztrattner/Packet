////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileLoader.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketFragment.h"
#include "PacketObjectManager.h"
#include "PacketFileLoaderQueue.h"

#include <thread>

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
class PacketFile;

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFileLoader
////////////////////////////////////////////////////////////////////////////////
class PacketFileLoader
{
public:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFileLoader(PacketObject* _packetObject);
	~PacketFileLoader();

//////////////////
// MAIN METHODS //
public: //////////

	// Process a packet file using the packet object
	bool ProcessPacketFile(PacketFile* _packetFile);

	// Set the file data
	bool GetFileData(PacketFragment::FileIdentifier _fileIdentifier, uint32_t& _fileSize, PacketObjectManager::FileFragmentIdentifier& _fileFragmentIdentifier);

private:

	// Load a file (thread safe)
	bool LoadFile(PacketFile* _file);
	
	// The threaded load routine
	void ThreadedLoadRoutine();

///////////////
// VARIABLES //
private: //////

	// Our packet object reference
	PacketObject* m_PacketObjectReference;

	// The load queue
	PacketFileLoaderQueue m_LoadQueue;

	// Or loading thread and mutex
	std::thread m_LoadingThread;
	std::mutex m_LoadingMutex;
};

// Packet data explorer
PacketNamespaceEnd(Packet)