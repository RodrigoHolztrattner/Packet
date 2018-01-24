////////////////////////////////////////////////////////////////////////////////
// Filename: PacketObjectHashTable.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketObjectManager.h"

#include <string>
#include <vector>

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

/*
	- Respons�vel por manter a estrutura fict�cia de pastas e arquivos dentro do nosso sistema.
	- Deve permitir fun��es do tipo: Put, Get, Delete, Insert, List, etc. Todas recebendo um path de entrada.
	- Os arquivos aqui devem ter uma hash que permita que os mesmos sejam localizados pelo Object Manager.
	- Pode ser um JSON.
*/

// Hash the given path (static)
static constexpr uint64_t HashFilePathStatic(const char* _filePath)
{
	return *_filePath ?
		static_cast<uint64_t>(*_filePath) + 33 * HashFilePathStatic(_filePath + 1) :
		5381;
}

// Hash the given path (non static)
static uint64_t HashFilePathNonStatic(const char* _filePath)
{
	return *_filePath ?
		static_cast<uint64_t>(*_filePath) + 33 * HashFilePathStatic(_filePath + 1) :
		5381;
}

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketObjectHashTable
////////////////////////////////////////////////////////////////////////////////
class PacketObjectHashTable
{
private:

public:

	// The packet object hash type
	typedef uint64_t PacketObjectHash;

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketObjectHashTable();
	~PacketObjectHashTable();

	// Initialize empty
	bool InitializeEmpty();

	// Initialize from data
	bool InitializeFromData(std::vector<unsigned char>& _data, uint32_t& _location);

	// Serialize
	std::vector<unsigned char> Serialize();

//////////////////
// MAIN METHODS //
public: //////////

	// Insert an entry <non static key>
	bool InsertEntry(std::string _path, PacketObjectManager::FileFragmentIdentifier& _internalIdentifier, PacketObjectHash& _hashOut);

	// Insert an entry <static key>
	bool InsertEntry(PacketObjectHash _key, PacketObjectManager::FileFragmentIdentifier& _internalIdentifier, PacketObjectHash& _hashOut);

	// Return an entry
	PacketObjectManager::FileFragmentIdentifier* GetEntry(std::string _path);

	// Return an entry (static hash)
	PacketObjectManager::FileFragmentIdentifier* GetEntry(PacketObjectHash _hash);

	// Remove an entry
	bool RemoveEntry(std::string _path);

	// Check if an entry exist
	bool EntryExist(std::string _path);

///////////////
// VARIABLES //
private: //////

	// The hash table
	std::map<PacketObjectHash, PacketObjectManager::FileFragmentIdentifier> m_HashTable;
};

// Packet data explorer
PacketNamespaceEnd(Packet)