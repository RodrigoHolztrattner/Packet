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

	// Return a file size
	unsigned int GetFileSize(PacketObjectManager::FileFragmentIdentifier* _fragmentFileIdentifier);

	// Return the file fragment identifier
	PacketObjectManager::FileFragmentIdentifier* GetFileFragmentIdentifier(PacketFragment::FileIdentifier _fileIdentifier);

private:

	// Load a file (thread safe)
	bool LoadFile(PacketFile* _file);

///////////////
// VARIABLES //
private: //////

	// Our packet object reference
	PacketObject* m_PacketObjectReference;

};

// Packet data explorer
PacketNamespaceEnd(Packet)