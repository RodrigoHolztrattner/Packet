////////////////////////////////////////////////////////////////////////////////
// Filename: PacketScanner.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"

#include <string>
#include <unordered_map>

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
// Class name: PacketScanner
////////////////////////////////////////////////////////////////////////////////
class PacketScanner
{
private:

	// The  folder node structure type
	struct FolderNode
	{
		~FolderNode()
		{
			for (auto& child : children)
			{
				delete child;
			}
		}

		// This folder path
		std::string folderPath;

		// A list with all file paths
		std::vector<std::string> files;

		// A list with all children folder nodes
		std::vector<FolderNode*> children;
	};

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketScanner();
	~PacketScanner();

//////////////////
// MAIN METHODS //
public: //////////

	// This method will scan for all folders and files inside the packet directory, creating our internal maps
	void Scan(std::string _packetDirectory);

	// This method will check if there are hash collisions between the files inside this packet, returning the
	// path for those files
	std::vector<std::string> CheckForHashCollisions();
	
	// Return a vector of pair<folder path, vector<pair<hash, file path>>> from our already processed data
	std::vector<std::pair<std::string, std::vector<std::pair<Hash, std::string>>>> GetFileTreeHashInfos();

	// Return the total number of folders/files
	uint32_t GetTotalFolderNumber();
	uint32_t GetTotalFileNumber();

///////////////
// VARIABLES //
private: //////

	// The unordered map of folders
	std::unordered_map<std::string, std::string> m_Folders;

	// The unordered map of files
	std::unordered_map<std::string, std::string> m_Files;

	// The base folder node
	FolderNode* m_RootFolderNode;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
