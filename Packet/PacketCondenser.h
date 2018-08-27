////////////////////////////////////////////////////////////////////////////////
// Filename: PacketCondenser.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"

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
PacketDevelopmentNamespaceBegin(Packet)

////////////////
// FORWARDING //
////////////////

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketCondenser
////////////////////////////////////////////////////////////////////////////////
class PacketCondenser
{
public:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketCondenser();
	~PacketCondenser();

//////////////////
// MAIN METHODS //
public: //////////

	// This method will receive a folder name and a vector vector of file hashes/paths and will package them all
	// together, returning the correspondent number of condensed file infos
	std::vector<CondensedFileInfo> CondenseFolder(std::string _rootPath, uint32_t _currentCondenseFileCount, std::vector<std::pair<Hash, std::string>>& _fileInfos);
	
///////////////
// VARIABLES //
private: //////


};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)