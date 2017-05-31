////////////////////////////////////////////////////////////////////////////////
// Filename: PacketDirectory.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"

#include <vector>
#include <string>
#include <map>

#include "PacketString.h"
#include "PacketFile.h"

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

class PacketManager;
class PacketIndexLoader;
class PacketLoader;

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketDirectory
////////////////////////////////////////////////////////////////////////////////
class PacketDirectory : public Serialize::Serializable
{
	friend PacketManager;
	friend PacketIndexLoader;
	friend PacketLoader;

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketDirectory();
	~PacketDirectory();

//////////////////
// MAIN METHODS //
public: //////////

	// Serialize this directory data
	std::vector<unsigned char> Serialize();

	// Deserialize this directory from the given data
	uint32_t Deserialize(std::vector<unsigned char>& _data, uint32_t _index);
	
///////////////
// VARIABLES //
private: //////


protected:

	// The current folder name
	PacketString folderName;

	// The current folder id
	uint32_t folderId;

	// All the child folders
	std::vector<PacketDirectory> childFolders;

	// All the child file info
	std::vector<PacketFile> childFileInfos;
};

// Packet data explorer
PacketNamespaceEnd(Packet)
