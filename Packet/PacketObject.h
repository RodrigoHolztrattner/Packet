////////////////////////////////////////////////////////////////////////////////
// Filename: PacketObject.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketFragment.h"
#include "PacketObjectManager.h"
#include "PacketObjectStructure.h"
#include "PacketObjectHashTable.h"
#include "PacketObjectIterator.h"

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

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketObject(std::string _packetName, uint32_t _maximumFragmentSize);
	~PacketObject();

	// Create this packet object
	// bool Create(std::string _packetObjectName, Attributes _packetObjectAttributes);

//////////////////
// MAIN METHODS //
public: //////////

	//
	PacketObjectIterator GetIterator();

	bool Save()
	{
		m_ObjectStructure.SaveObjectStructure();
		return true;
	}

///////////////
// VARIABLES //
private: //////

	// The packet object name
	std::string m_PacketObjectName;

	// The packet object attributes
	// Attributes m_PacketObjectAttributes;

	 // The number of fragments this object have
	uint32_t m_TotalNumberFragments;

	// The total number of files inside this packet
	uint32_t m_TotalNumberFiles;

	// The current internal identifier number
	uint32_t m_CurrentInternalIdentifierNumber;

	// Our object manager, structure and hash table
	PacketObjectManager m_ObjectManager;
	PacketObjectStructure m_ObjectStructure;
	PacketObjectHashTable m_ObjectHashTable;
};

// Packet data explorer
PacketNamespaceEnd(Packet)
