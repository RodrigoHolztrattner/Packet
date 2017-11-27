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
	static const char FolderDelimiterType = '\\';

	// The dot type
	static const char FileDelimiterType = '.';

	// The back type
	static const std::string BackDelimiterType;

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
	static std::string GetDirectoryFromPath(std::string& _path);

	// Return the filename from the given path
	static std::string GetFilenameFromPath(std::string& _path);

	// Split the given path
	static std::vector<std::string> SplitPath(std::string& str);

	// Compose a string dir from the given dir
	static std::string ComposeDirectory(std::vector<std::string>& _dir);

	// Join a directory with a seek operation
	static std::vector<std::string> JoinDirectorySeek(std::vector<std::string>& _dir, std::vector<std::string>& _seek);

	// Check if the given path is a file path
	static bool PathIsFile(std::string _path);

	// Check if the given path is a folder path
	static bool PathIsFolder(std::string& _path, bool _ignoreBackDelimiter = true);

///////////////
// VARIABLES //
private: //////

};

// Packet data explorer
PacketNamespaceEnd(Packet)
