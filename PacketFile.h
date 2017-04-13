////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFile.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "..\NamespaceDefinitions.h"
#include "..\Serialize.h"
#include "PacketString.h"

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

class PacketManager;
class PacketIndexLoader;
class PacketLoader;

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFile
////////////////////////////////////////////////////////////////////////////////
class PacketFile : public Serialize::Serializable
{
public:

	friend PacketManager;
	friend PacketIndexLoader;
	friend PacketLoader;

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFile();
	~PacketFile();

//////////////////
// MAIN METHODS //
public: //////////

	// Serialize this file data
	std::vector<unsigned char> Serialize();

	// Deserialize this file from the given data
	uint32_t Deserialize(std::vector<unsigned char>& _data, uint32_t _index);

	// Return the identifier
	uint32_t GetIdentifier() { return fileId; }

///////////////
// VARIABLES //
private: //////

	// The file name
	PacketString fileName;

	// The file id
	uint32_t fileId;

	// The file path
	PacketString fileExternalPath;
};

// Packet data explorer
NamespaceEnd(Packet)