////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileStorage.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketError.h"
#include "PacketFile.h"

#include <map>

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
// Class name: PacketFileStorage
////////////////////////////////////////////////////////////////////////////////
class PacketFileStorage
{

private:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFileStorage();
	~PacketFileStorage();

//////////////////
// MAIN METHODS //
public: //////////

	// Insert a file with the given identifier
	bool InserFileWithIdentifier(PacketFragment::FileIdentifier _fileIdentifier, PacketFile* _file);

	// Request a file from the given identifier
	PacketFile* RequestFileFromIdentifier(PacketFragment::FileIdentifier _fileIdentifier);

	// Try to release a file from the given identifier, return a ptr to the file if it should be deleted
	PacketFile* TryReleaseFileFromIdentifier(PacketFragment::FileIdentifier _fileIdentifier);

private:

///////////////
// VARIABLES //
private: //////

	// Our file x identifier storage
	std::map<PacketFragment::FileIdentifier, PacketFile*> m_FileStorage; // TODO usar algum vetor com memory allocation diferente? Setar size inicial? Fazer com que ele nunca recue no size?
};

// Packet data explorer
PacketNamespaceEnd(Packet)