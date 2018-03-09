////////////////////////////////////////////////////////////////////////////////
// Filename: PacketObjectIteratorPath.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketObjectStructure.h"

#include <vector>
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
PacketNamespaceBegin(Packet)

////////////////
// FORWARDING //
////////////////

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketObjectIteratorPath
////////////////////////////////////////////////////////////////////////////////
class PacketObjectIteratorPath
{
public:

	// The path type
	enum class PathType
	{
		File,
		Directory
	};

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketObjectIteratorPath(PacketObjectStructure& _packetStructureReference);
	~PacketObjectIteratorPath();

//////////////////
// MAIN METHODS //
public: //////////

	// Compose a seek directory
	bool ComposeSeek(std::string _path);

	// Return the current path and action path
	std::string GetCurrentPath();
	std::vector<std::string>& GetCurrentActionPath();

	// Compose the action directory from the given path
	std::vector<std::string> ComposeActionDirectory(std::string& _path, bool _seeking = false);

private:

	// Set the current path
	void SetCurrentPath(std::vector<std::string> _currentPath);

///////////////
// VARIABLES //
private: //////

	// The packet structure reference
	PacketObjectStructure& m_PacketStructureReference;

	// The current directory path
	std::vector<std::string> m_CurrentDirectoryPath;
};

// Packet data explorer
PacketNamespaceEnd(Packet)
