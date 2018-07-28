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
PacketDevelopmentNamespaceBegin(Packet)

////////////////
// FORWARDING //
////////////////

////////////////
// STRUCTURES //
////////////////

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

	// Seeks to the given path
	bool Seek(std::string _path);

	// Create a directory
	bool MakeDir(std::string _dirPath);

	// Put a file/data <inside the current path>
	bool Put(unsigned char* _data, uint32_t _size);
	bool Put(unsigned char* _data, uint32_t _size, std::string _iFolderLocation);
	bool Put(std::string _filePath, std::string _iFolderLocation);
	bool Put(std::string _filePath);

	// Get a file/data <from the current path>
	bool Get(std::string _iFileLocation, unsigned char* _data);
	bool Get(std::string _iFileLocation);								
	bool Get(std::string _iFileLocation, std::string _oFileLocation);	

	// Deletes the current path <the input path>
	bool Delete(std::string _iLocation);

	// Optimizes the internal fragments
	bool Optimize();

	// Get a list of each folder and file from the current path or from the input path
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
PacketDevelopmentNamespaceEnd(Packet)
