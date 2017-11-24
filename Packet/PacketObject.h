////////////////////////////////////////////////////////////////////////////////
// Filename: PacketObject.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketFragment.h"

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
// Class name: PacketObject
////////////////////////////////////////////////////////////////////////////////
class PacketObject
{
public:

	// The attributes
	struct Attributes
	{
		// The maximum fragment size (this cannot be changed after set)
		uint32_t maximumFragmentSize;
	};

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketObject();
	~PacketObject();

	// Create this packet object
	bool Create(std::string _packetObjectName, Attributes _packetObjectAttributes);

//////////////////
// MAIN METHODS //
public: //////////

///////////////
// VARIABLES //
private: //////

	// The packet object name
	std::string m_PacketObjectName;

	// The packet object attributes
	Attributes m_PacketObjectAttributes;

	 // The number of fragments this object have
	uint32_t m_TotalNumberFragments;

	// The total number of files inside this packet
	uint32_t m_TotalNumberFiles;

	// The current internal identifier number
	uint32_t m_CurrentInternalIdentifierNumber;
};

// Packet data explorer
PacketNamespaceEnd(Packet)
