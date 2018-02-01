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
#include "PacketObjectIteratorPath.h"
#include "PacketObjectTemporaryPath.h"
#include "PacketError.h"

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

	// Create a dir <inside the current path>
	bool MakeDir(std::string _dirPath);

	// Put a file/data <inside the current path>
	bool Put(unsigned char* _data, uint32_t _size);
	bool Put(unsigned char* _data, uint32_t _size, std::string iFolderLocation);
	bool Put(std::string _filePath, std::string iFolderLocation);		// External location to get / internal path to put
	bool Put(std::string _filePath);									// External location to get

	// Get a file/data <from the current path>
	bool Get(std::string _iFileLocation, unsigned char* _data, uint32_t _size);
	bool Get(std::string _iFileLocation);								// Internal location to get
	bool Get(std::string _iFileLocation, std::string _oFileLocation);	// Internal location to get / external location to put 

	// Delete the current path <file>
	bool Delete(std::string _iLocation);

	// Otimize the internal fragments
	bool Otimize();

	// Get a list of each folder and file from the given path
	std::vector<std::string> List();
	std::vector<std::string> List(std::string _path);

	// Return the current path
	std::string GetCurrentPath();

	// Return the error object
	PacketError GetError();

private:

	// Put a file <inside the given path> aux
	bool PutAux(PacketObjectTemporaryPath& _temporaryPath);

	// Get a file <from the given path> aux
	bool GetAux(PacketObjectTemporaryPath& _temporaryPath);

	// Delete a file/folder recursivelly
	bool DeleteFile(std::string _iFileLocation);
	bool DeleteFolder(std::string _iFolderLocation);

private:

///////////////
// VARIABLES //
private: //////

	// The iterator path
	PacketObjectIteratorPath m_IteratorPath;

	// The packet object manager reference
	PacketObjectManager& m_PacketManagerReference;

	// The packet structure reference
	PacketObjectStructure& m_PacketStructureReference;

	// The packet hash table reference
	PacketObjectHashTable& m_PacketHashTableReference;

	// The error object
	PacketError m_ErrorObject;
};

// Packet data explorer
PacketNamespaceEnd(Packet)
