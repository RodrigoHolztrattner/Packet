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
#include "PacketFileLoader.h"

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

// We know the PacketFileLoader and the PacketFile classes
class PacketFileLoader;
class PacketFile;

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketObject
////////////////////////////////////////////////////////////////////////////////
class PacketObject
{
public:

	// The PacketFileLoader and the PacketFile are friend classes
	friend PacketFileLoader;
	friend PacketFile;

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

//////////////////
// MAIN METHODS //
public: //////////

	// Return this packet object iterator
	PacketObjectIterator GetIterator();

private:

	// Save this packet data aux
	bool SavePacketDataAux(std::string _filePath);

	// Return our manager, structure and hash table reference
	PacketObjectManager* GetObjectManagerReference();
	PacketObjectStructure* GetObjectStructureReference();
	PacketObjectHashTable* GetObjectHashTableReference();

///////////////
// VARIABLES //
private: //////

	// The packet object data
	std::string m_PacketObjectName;
	uint32_t m_MaximumFragmentSize;
	uint32_t m_TotalNumberFragments;
	uint32_t m_TotalNumberFiles;

	// The oppened file path
	std::string m_OppenedFilePath;

	// The current internal identifier number
	uint32_t m_CurrentInternalIdentifierNumber;

	// Our object manager, structure and hash table
	PacketObjectManager m_ObjectManager;
	PacketObjectStructure m_ObjectStructure;
	PacketObjectHashTable m_ObjectHashTable;
};

// Packet data explorer
PacketNamespaceEnd(Packet)
