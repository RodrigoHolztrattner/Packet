////////////////////////////////////////////////////////////////////////////////
// Filename: PacketObjectStructure.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketFragment.h"
#include "PacketObjectManager.h"
#include "PacketObjectHashTable.h"

#include <string>
#include <vector>

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
// Class name: PacketObjectStructure
////////////////////////////////////////////////////////////////////////////////
class PacketObjectStructure
{
private:

	// The file object type
	struct FileObjectType
	{
		// This file name
		std::string fileName;

		// This file extension
		std::string fileExtension;

		// The file complete path
		std::string filePath;

		// The file hash identifier
		PacketObjectHashTable::PacketObjectHash fileHashIdentifier;
	};

	// The folder object type
	struct FolderObjectType
	{
		// This folder name
		std::string folderName;

		// The folder complete path
		std::string folderPath; // Not used

		// All the file objects this folder has
		std::vector<FileObjectType*> files;

		// All the sub-folder this folder has
		std::vector<FolderObjectType*> subFolders;
	};

public:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketObjectStructure();
	~PacketObjectStructure();

	// Initialize empty
	bool InitializeEmpty(std::string _packetName);

	// Initialize from data
	bool InitializeFromData(std::vector<unsigned char>& _data, uint32_t& _location, std::string _packetName);

	// Serialize
	std::vector<unsigned char> Serialize();

//////////////////
// MAIN METHODS //
public: //////////

	// Insert/remove a new folder inside the given directory
	bool InsertFolder(std::string _folderName, std::vector<std::string>& _directoryPath);
	bool RemoveFolder(std::string _folderName, std::vector<std::string>& _directoryPath);

	// Insert/remove a file inside the given directory
	bool InsertFile(std::string _fileName, PacketObjectHashTable::PacketObjectHash _fileHashIdentifier, std::vector<std::string>& _directoryPath);
	bool RemoveFile(std::string _fileName, std::vector<std::string>& _directoryPath);

	// Check if the given directory path is valid
	bool DirectoryFromPathIsValid(std::vector<std::string>& _directoryPath);
	bool DirectoryFromPathIsValid(std::vector<std::string>& _directoryPath, std::string _directoryName);

	// Check if a folder/file from the given path exist
	bool FileFromPathIsValid(std::vector<std::string>& _directoryPath, std::string _fileName);
	bool FolderFromPathIsValid(std::vector<std::string>& _directoryPath);

	// Return a list with all folders/files from the given directory
	std::vector<std::string> GetFolderList(std::vector<std::string>& _directoryPath);
	std::vector<std::string> GetFileList(std::vector<std::string>& _directoryPath);

	// Return the root folder name
	std::string GetRootName();

private:

	// Return a folder from the given path
	FolderObjectType* GetFolderFromDirectory(std::vector<std::string>& _directoryPath, bool _returnParentFolder = false);

	// Return a file from the given path and filename
	FileObjectType* GetFileFromDirectory(std::vector<std::string>& _directoryPath, std::string _fileName);

	// Check if a folder has a sub folder with the given name
	bool FolderHasSubFolder(FolderObjectType* _folder, std::string _folderName);

	// Check if a folder has a child file with the given name
	bool FolderHasFile(FolderObjectType* _folder, std::string _fileName);

public:

	// Load this object structure
	void LoadObjectStructureAux(FolderObjectType* _folder, std::vector<unsigned char>& _data, uint32_t& _location);

	// Save this object structure
	void SaveObjectStructureAux(FolderObjectType* _folder, std::vector<unsigned char>& _data, uint32_t& _location);

///////////////
// VARIABLES //
private: //////

	// If this was initialized
	bool m_Initialized;

	// The root folder object
	FolderObjectType* m_RootFolder;

	// The packet object name
	std::string m_PacketObjectName;
};

// Packet data explorer
PacketNamespaceEnd(Packet)
