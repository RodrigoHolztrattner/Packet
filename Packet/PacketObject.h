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
	PacketObject();
	~PacketObject();

	// Initialize this packet empty
	bool InitializeEmpty(std::string _packetName, uint32_t _maximumFragmentSize);

	// Initialize this packet from a file
	bool InitializeFromFile(std::string _filePath);

	// Save this packet data
	bool SavePacketData();
	bool SavePacketData(std::string _filePath);

//////////////////
// MAIN METHODS //
public: //////////

	// Return this packet object iterator
	PacketObjectIterator GetIterator();

///////////////
// VARIABLES //
private: //////

	// The packet object name
	std::string m_PacketObjectName;

	// The maximum fragment size
	uint32_t m_MaximumFragmentSize;

	// The oppened file path
	std::string m_OppenedFilePath;

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
