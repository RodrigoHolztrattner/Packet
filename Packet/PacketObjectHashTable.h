////////////////////////////////////////////////////////////////////////////////
// Filename: PacketObjectHashTable.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketObjectStructure.h"
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
	- Responsável por manter a estrutura fictícia de pastas e arquivos dentro do nosso sistema.
	- Deve permitir funções do tipo: Put, Get, Delete, Insert, List, etc. Todas recebendo um path de entrada.
	- Os arquivos aqui devem ter uma hash que permita que os mesmos sejam localizados pelo Object Manager.
	- Pode ser um JSON.
*/

// Hash the given path (static)
static constexpr uint32_t HashFilePathStatic(const char* _filePath)
{
	return *_filePath ?
		static_cast<uint32_t>(*_filePath) + 33 * HashFilePathStatic(_filePath + 1) :
		5381;
}

// Hash the given path (non static)
static uint32_t HashFilePathNonStatic(const char* _filePath)
{
	return *_filePath ?
		static_cast<uint32_t>(*_filePath) + 33 * HashFilePathStatic(_filePath + 1) :
		5381;
}

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketObjectHashTable
////////////////////////////////////////////////////////////////////////////////
class PacketObjectHashTable
{
private:

	// The hash internal representation
	struct HashInternal
	{
		// The file fragment identifier
		PacketObjectManager::FileFragmentIdentifier fragmentIdentifier;

		// The path string
		std::string fullPath;
	};


public:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketObjectHashTable();
	~PacketObjectHashTable();

//////////////////
// MAIN METHODS //
public: //////////

	// Insert an entry
	uint32_t InsertEntry(std::string _path, PacketObjectManager::FileFragmentIdentifier& _internalIdentifier);

	// Remove an entry
	bool RemoveEntry(std::string _path);

	// Return an entry
	PacketObjectManager::FileFragmentIdentifier* GetEntry(std::string _path);

	// Return an entry (static hash)
	PacketObjectManager::FileFragmentIdentifier* GetEntry(const std::string _path, uint32_t _hash);

///////////////
// VARIABLES //
private: //////

	// The hash table
	std::map<uint32_t, std::vector<HashInternal>> m_HashTable;
};

// Packet data explorer
PacketNamespaceEnd(Packet)
