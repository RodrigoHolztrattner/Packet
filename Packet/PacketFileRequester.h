////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileRequester.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketError.h"

#include <string>

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

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFileRequester
////////////////////////////////////////////////////////////////////////////////
class PacketFileRequester
{
public:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFileRequester(PacketObject* _packetObject);
	~PacketFileRequester();

//////////////////
// MAIN METHODS //
public: //////////

	// Initialize this file requester
	bool Initialize();

	// Set to use multiple thread structures
	bool UseThreadStructures(uint32_t _totalNumberMaximumThreads, std::function<uint32_t> _threadIndexMethod);

protected:

///////////////
// VARIABLES //
private: //////

	// If we are using the thread loading structure mode (we will use multiples arrays
	// (one for each thread) to allow multiple simultaneous access without mutex uses)
	bool m_UseThreadStructures;

	// Our PacketObject reference
	PacketObject* m_PacketObjectReference;
};

// Packet data explorer
PacketNamespaceEnd(Packet)