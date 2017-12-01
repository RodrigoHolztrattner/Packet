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

/*
	- Responsável por manter a estrutura fictícia de pastas e arquivos dentro do nosso sistema.
	- Deve permitir funções do tipo: Put, Get, Delete, Insert, List, etc. Todas recebendo um path de entrada.
	- Os arquivos aqui devem ter uma hash que permita que os mesmos sejam localizados pelo Object Manager.
	- Pode ser um JSON.
*/

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketObjectStructure
////////////////////////////////////////////////////////////////////////////////
class PacketObjectStructure
{
private:

	// The structure extension type
	const std::string StructureExtensionType = ".structure";

	// The file object type
	struct FileObjectType
	{
		// This file name
		std::string fileName;

		// The file complete path
		std::string filePath;

		// The file hash identifier
		uint32_t fileHashIdentifier;
	};

	// The folder object type
	struct FolderObjectType
	{
		// This folder name
		std::string folderName;

		// The folder complete path
		std::string folderPath;

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
	PacketObjectStructure(std::string _packetName);
	~PacketObjectStructure();

//////////////////
// MAIN METHODS //
public: //////////

	// Insert a new folder inside the given directory
	bool InsertFolder(std::string _folderName, std::vector<std::string>& _directoryPath);

	// Insert a file inside the given directory
	bool InsertFile(std::string _fileName, uint32_t _fileHashIdentifier, std::vector<std::string>& _directoryPath);

	// Check if the given directory path is valid
	bool DirectoryFromPathIsValid(std::vector<std::string>& _directoryPath);
	bool DirectoryFromPathIsValid(std::vector<std::string>& _directoryPath, std::string _directoryName);

	// Check a file from the given path exist
	bool FileFromPathIsValid(std::vector<std::string>& _directoryPath, std::string _fileName);

	// Return a list with all folders and files from the given directory
	std::vector<std::string> GetFolderList(std::vector<std::string>& _directoryPath);

	// Return the root folder name
	std::string GetRootName();

private:

	// Return a folder from the given path
	FolderObjectType* GetFolderFromDirectory(std::vector<std::string>& _directoryPath);

	// Return a file from the given path and filename
	FileObjectType* GetFileFromDirectory(std::vector<std::string>& _directoryPath, std::string _fileName);

	// Check if a folder has a sub folder with the given name
	bool FolderHasSubFolder(FolderObjectType* _folder, std::string _folderName);

	// Check if a folder has a child file with the given name
	bool FolderHasFile(FolderObjectType* _folder, std::string _fileName);

private:
public:

	// Load this object structure
	bool LoadObjectStructure();
	bool LoadObjectStructureAux(FolderObjectType* _folder, std::ifstream& _ofstream);

	// Save this object structure
	bool SaveObjectStructure();
	bool SaveObjectStructureAux(FolderObjectType* _folder, std::ofstream& _ofstream);

///////////////
// VARIABLES //
private: //////

	// The root folder object
	FolderObjectType* m_RootFolder;

	// The packet object name
	std::string m_PacketObjectName;
};

// Packet data explorer
PacketNamespaceEnd(Packet)
