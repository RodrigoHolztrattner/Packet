////////////////////////////////////////////////////////////////////////////////
// Filename: PacketIndex.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "..\NamespaceDefinitions.h"
#include "..\HashedString.h"
#include "PacketString.h"
#include "PacketFile.h"
#include "PacketDirectory.h"

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
NamespaceBegin(Packet)

////////////////
// FORWARDING //
////////////////

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketIndex
////////////////////////////////////////////////////////////////////////////////
class PacketIndex
{
//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketIndex();
	~PacketIndex();

//////////////////
// MAIN METHODS //
public: //////////

///////////////
// VARIABLES //
public: ///////

	// The total number of files
	uint32_t totalNumberFiles;

	// The total number of folders
	uint32_t totalNumberFolders;

	// The folder/file identifier references
	std::map<HashedStringIdentifier, PacketDirectory*> folderIdentifierReferences; //TODO: Usar o map mais rapido
	std::map<HashedStringIdentifier, PacketFile*> fileIdentifierReferences;

	// The folder/file identifier freelist
	std::vector<HashedStringIdentifier> folderIdentifierFreelist;
	std::vector<HashedStringIdentifier> fileIdentifierFreelist;
};

// Packet data explorer
NamespaceEnd(Packet)