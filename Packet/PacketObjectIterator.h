////////////////////////////////////////////////////////////////////////////////
// Filename: PacketObjectIterator.h
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

#include <string>
#include <stack>

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
	- Responsável por controlar a saida e entrada de arquivos/pastas
	- Apenas uma classe de ajuda (teoricamente)
	- Deve fazer chamadas ao object manager (que tem as informações de onde está cada arquivo/pasta) e ao object structure

*/

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketObjectIterator
////////////////////////////////////////////////////////////////////////////////
class PacketObjectIterator
{
public:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketObjectIterator(PacketObjectManager& _packetManagerReference, PacketObjectStructure& _packetStructureManager, PacketObjectHashTable& _packetHashTableReference);
	~PacketObjectIterator();

//////////////////
// MAIN METHODS //
public: //////////

	// Seek to the given path
	bool Seek(std::string _path);

	// Put a file/data <inside the current path>
	bool Put(unsigned char* _data, uint32_t _size);
	bool Put(unsigned char* _data, uint32_t _size, std::string iFolderLocation);
	bool Put(std::string _filePath, std::string iFolderLocation);
	bool Put(std::string _filePath);

	// Create a dir <inside the current path>
	bool MakeDir(std::string _dirPath);

	// Get a file/data <from the current path>
	bool Get(std::string _iFileLocation, unsigned char* _data, uint32_t _size);
	bool Get(std::string _iFileLocation);
	bool Get(std::string _iFileLocation, std::string _oFileLocation);

	// Delete the current path <file>
	bool Delete(std::string _path);

	// Get a list of each folder and file from the given path
	std::vector<std::string> List();
	std::vector<std::string> List(std::string _path);

	// Return the current path
	std::string GetCurrentPath();

private:

	// Put a file <inside the given path> aux
	bool PutAux(std::string _filePath, std::string _fileName, std::vector<std::string> _dir, std::string _stringDir);

	// Get a file <from the given path> aux
	bool GetAux(std::string _internalFilePath, std::string _fileName, std::vector<std::string> _dir, std::string _outputFilePath);

private:

	// Compose the action directory from the given path
	std::vector<std::string> ComposeActionDirectory(std::string& _path, bool _seeking = false);

///////////////
// VARIABLES //
private: //////

	// The current directory path
	std::vector<std::string> m_CurrentDirectoryPath;

	// The packet object manager reference
	PacketObjectManager& m_PacketManagerReference;

	// The packet structure reference
	PacketObjectStructure& m_PacketStructureReference;

	// The packet hash table reference
	PacketObjectHashTable& m_PacketHashTableReference;
};

// Packet data explorer
PacketNamespaceEnd(Packet)
