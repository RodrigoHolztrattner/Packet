////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketObjectStructure.h"

Packet::PacketObjectStructure::PacketObjectStructure()
{
	// Set the initial data
	// ...

	// Create the root folder
	m_RootFolder = new FolderObjectType;
	m_RootFolder->folderName = "";
}

Packet::PacketObjectStructure::~PacketObjectStructure()
{
}

bool Packet::PacketObjectStructure::InsertFolder(std::string _folderName, std::vector<std::string>& _directoryPath)
{
	// Try to get the folder
	FolderObjectType* folder = GetFolderFromDirectory(_directoryPath);
	if (folder == nullptr)
	{
		return false;
	}

	// Check if the current folder already have a child with the given name
	if (FolderHasSubFolder(folder, _folderName))
	{
		return false;
	}

	// Create the new folder object
	FolderObjectType* newFolder = new FolderObjectType();
	newFolder->folderName = _folderName;
	// newFolder->folderPath = 

	// Insert the new folder inside the base one
	folder->subFolders.push_back(newFolder);

	return true;
}

bool Packet::PacketObjectStructure::InsertFile(std::string _fileName, uint32_t _fileHashIdentifier, std::vector<std::string>& _directoryPath)
{
	// Try to get the folder
	FolderObjectType* folder = GetFolderFromDirectory(_directoryPath);
	if (folder == nullptr)
	{
		return false;
	}

	// Check if the current folder already have a child file with the given name
	if (FolderHasFile(folder, _fileName))
	{
		return false;
	}

	// Create the new file object
	FileObjectType* newFile = new FileObjectType();
	newFile->fileName = _fileName;
	newFile->fileHashIdentifier = _fileHashIdentifier;
	// newFile->filePath = 

	// Insert the new file inside the current folder
	folder->files.push_back(newFile);

	return true;
}

bool Packet::PacketObjectStructure::DirectoryFromPathIsValid(std::vector<std::string>& _directoryPath)
{
	// Try to get the folder
	if (GetFolderFromDirectory(_directoryPath) == nullptr)
	{
		return false;
	}

	return true;
}

bool Packet::PacketObjectStructure::FileFromPathIsValid(std::vector<std::string>& _directoryPath, std::string _fileName)
{
	// Try to get the folder
	if (GetFileFromDirectory(_directoryPath, _fileName) == nullptr)
	{
		return false;
	}

	return true;
}

Packet::PacketObjectStructure::FolderObjectType* Packet::PacketObjectStructure::GetFolderFromDirectory(std::vector<std::string>& _directoryPath)
{
	// For each folder inside the path
	FolderObjectType* currentFolder = m_RootFolder;
	for (auto& directoryName : _directoryPath)
	{
		// If we found the directory
		bool found = false;

		// For each folder inside the current one
		for (auto folder : currentFolder->subFolders)
		{
			// Compare the names
			if (directoryName.compare(folder->folderName) == 0)
			{
				// Set the new current folder
				currentFolder = folder;

				// Set found to true
				found = true;

				break;
			}
		}

		// Check if we found the directory
		if (!found)
		{
			// Invalid path
			return nullptr;
		}
	}

	return currentFolder;
}

Packet::PacketObjectStructure::FileObjectType* Packet::PacketObjectStructure::GetFileFromDirectory(std::vector<std::string>& _directoryPath, std::string _fileName)
{
	// Try to get the folder
	FolderObjectType* folder = GetFolderFromDirectory(_directoryPath);
	if (folder == nullptr)
	{
		return nullptr;
	}

	// For each file inside this folder
	for (unsigned int i = 0; i < folder->files.size(); i++)
	{
		// Compare the names
		if (folder->files[i]->fileName.compare(_fileName) == 0)
		{
			return folder->files[i];
		}
	}

	return nullptr;
}

bool Packet::PacketObjectStructure::FolderHasSubFolder(FolderObjectType* _folder, std::string _folderName)
{
	// For each subfolder inside this folder
	for (auto folder : _folder->subFolders)
	{
		// Compare the names
		if (_folderName.compare(folder->folderName) == 0)
		{
			return true;
		}
	}

	return false;
}

bool Packet::PacketObjectStructure::FolderHasFile(FolderObjectType* _folder, std::string _fileName)
{
	// For each file inside this folder
	for (auto file : _folder->files)
	{
		// Compare the names
		if (_fileName.compare(file->fileName) == 0)
		{
			return true;
		}
	}

	return false;
}