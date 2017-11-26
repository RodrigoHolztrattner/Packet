////////////////////////////////////////////////////////////////////////////////
// Filename: PacketStringOperations.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketFragment.h"

#include <string>
#include <vector>
#include <set>

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
// Class name: PacketStringOperations
////////////////////////////////////////////////////////////////////////////////
class PacketStringOperations
{
private:

	// The slash type
	static const char DelimiterType = '\\';

public:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketStringOperations();
	~PacketStringOperations();

//////////////////
// MAIN METHODS //
public: //////////

	// Return the directory from the given path
	static std::string GetDirectoryFromPath(std::string _path);

	// Return the filename from the given path
	static std::string GetFilenameFromPath(std::string _path);

	// Split the given path
	static std::vector<std::string> SplitPath(std::string& str);

	// Compose a string dir from the given dir
	static std::string ComposeDirectory(std::vector<std::string> _dir);

///////////////
// VARIABLES //
private: //////

};

// Packet data explorer
PacketNamespaceEnd(Packet)
