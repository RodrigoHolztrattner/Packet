////////////////////////////////////////////////////////////////////////////////
// Filename: PacketObjectTemporaryPath.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketObjectStructure.h"
#include "PacketObjectIteratorPath.h"

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
// Class name: PacketObjectTemporaryPath
////////////////////////////////////////////////////////////////////////////////
class PacketObjectTemporaryPath
{
public:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketObjectTemporaryPath(PacketObjectStructure& _packetStructureReference, PacketObjectIteratorPath& _packetIteratorPathReference);
	~PacketObjectTemporaryPath();

//////////////////
// MAIN METHODS //
public: //////////

	// Compose a temporary path
	bool ComposeTemporaryPath(std::string _appendPath);
	bool ComposeTemporaryPath(std::vector<std::string> _appendPath);

	// Insert a external file path
	bool InsertExternalFilePath(std::string _externalFilePath);

	// Merge the external file name with the current temporary path
	bool MergeExternalFileName();

public:

	// Return the full path or split path
	std::string GetFullPath();
	std::vector<std::string> GetFullSplitPath();

	// Return the folder path or split path
	std::string GetFolderPath();
	std::vector<std::string> GetFolderSplitPath();

	// Return the file name with extension / without extension / extension only
	std::string GetFilename();
	std::string GetFileWithoutExtension();
	std::string GetFileExtension();

	// If the current path is a file / folder
	bool IsFile();
	bool IsFolder();

	// If the current path is valid
	bool IsValid();

	// Return the external file path / folder path / file name
	std::string GetExternalFilePath();
	std::string GetExternalFolderPath();
	std::string GetExternalFilename();

private:

	// Compose a temporary path aux
	bool ComposeTemporaryPathAux(std::vector<std::string> _appendPath);

	// Try to join the input path with the current one in the iterator path reference
	bool TryJoinWithCurrentPath(std::vector<std::string> _appendPath);

	// Try to use the input path as an absolute one
	bool TryUseAbsolutePath(std::vector<std::string> _appendPath);

private:


///////////////
// VARIABLES //
private: //////

	// The packet structure and iterator path references
	PacketObjectStructure& m_PacketStructureReference;
	PacketObjectIteratorPath& m_PacketIteratorPathReference;

	// The full split path (contains any filename if there is one)
	std::vector<std::string> m_FullSplitPath;

	// The folder split path (only contain folder names)
	std::vector<std::string> m_FolderSplitPath;

	// The file name and extension (if the current path is a file)
	std::string m_FileName; // With extension
	std::string m_FileNameNoExtension;
	std::string m_FileExtension;

	// Boolean checks
	bool m_IsFile;
	bool m_IsFolder;
	bool m_IsValid;

	// The external file path / folder path /file name 
	std::string m_ExternalFilePath;
	std::string m_ExternalFolderPath;
	std::string m_ExternalFileName;
};

// Packet data explorer
PacketNamespaceEnd(Packet)
