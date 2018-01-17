////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketObjectStructure.h"
#include "PacketFileDataOperations.h"
#include "PacketStringOperations.h"

#include <experimental/filesystem>

Packet::PacketObjectStructure::PacketObjectStructure()
{
	// Set the initial data
	m_Initialized = false;
}

Packet::PacketObjectStructure::~PacketObjectStructure()
{

}

bool Packet::PacketObjectStructure::InitializeEmpty(std::string _packetName)
{
	// Set the packet name
	m_PacketObjectName = _packetName;

	// Create the default root folder
	m_RootFolder = new FolderObjectType;
	m_RootFolder->folderName = m_PacketObjectName;

	// Set initialized
	m_Initialized = true;

	return true;
}

bool Packet::PacketObjectStructure::InitializeFromData(std::vector<unsigned char>& _data, uint32_t& _location, std::string _packetName)
{
	// Check if the data is valid
	if (!_data.size())
	{
		return false;
	}

	// Set the packet name
	m_PacketObjectName = _packetName;

	// Create the root folder
	m_RootFolder = new FolderObjectType;

	// Load the object structure
	LoadObjectStructureAux(m_RootFolder, _data, _location);

	// Set initialized
	m_Initialized = true;

	return true;
}

std::vector<unsigned char> Packet::PacketObjectStructure::Serialize()
{
	// The output data
	std::vector<unsigned char> outData;
	uint32_t location = 0;

	// Save the object structure
	SaveObjectStructureAux(m_RootFolder, outData, location);

	return outData;
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

bool Packet::PacketObjectStructure::RemoveFolder(std::string _folderName, std::vector<std::string>& _directoryPath)
{
	// Try to get the folder
	FolderObjectType* folder = GetFolderFromDirectory(_directoryPath);
	if (folder == nullptr)
	{
		return false;
	}

	// Try to get the parent folder
	FolderObjectType* parentFolder = GetFolderFromDirectory(_directoryPath, true);
	if (parentFolder == nullptr)
	{
		return false;
	}

	// Check if we are trying to delete the root folder
	if (folder == parentFolder)
	{
		// We can't delete the root folder
		return false;
	}

	// Check if there are any filder os sub-folders for the given folder
	if (folder->subFolders.size() > 0 || folder->files.size() > 0)
	{
		// We can't delete this folder because it isn't empty
		return false;
	}

	// For each subfolder inside the parent folder
	for (int i = 0; i < parentFolder->subFolders.size(); i++)
	{
		// Get the current folder
		FolderObjectType* currentFolder = parentFolder->subFolders[i];

		// Compare the names
		if (_folderName.compare(currentFolder->folderName) == 0)
		{
			// Delete this folder
			delete currentFolder;

			// Remove this folder from the parent one
			parentFolder->subFolders.erase(parentFolder->subFolders.begin() + i);

			return true;
		}
	}
	
	return false;
}

bool Packet::PacketObjectStructure::InsertFile(std::string _fileName, PacketObjectHashTable::PacketObjectHash _fileHashIdentifier, std::vector<std::string>& _directoryPath)
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
	newFile->fileExtension = std::experimental::filesystem::path(_fileName).extension().string();
	newFile->fileHashIdentifier = _fileHashIdentifier;
	newFile->filePath = PacketStringOperations::ComposeDirectory(_directoryPath);

	// Insert the new file inside the current folder
	folder->files.push_back(newFile);

	return true;
}

bool Packet::PacketObjectStructure::RemoveFile(std::string _fileName, std::vector<std::string>& _directoryPath)
{
	// Try to get the folder
	FolderObjectType* folder = GetFolderFromDirectory(_directoryPath);
	if (folder == nullptr)
	{
		return false;
	}

	// For each file inside this folder
	for (int i=0; i<folder->files.size(); i++)
	{
		// Get the current file
		FileObjectType* file = folder->files[i];

		// Compare the names
		if (_fileName.compare(file->fileName) == 0)
		{
			// Delete this file
			delete file;

			// Remove this file
			folder->files.erase(folder->files.begin() + i);

			return true;
		}
	}

	// There is no file with the given filename here
	return false;
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

bool Packet::PacketObjectStructure::DirectoryFromPathIsValid(std::vector<std::string>& _directoryPath, std::string _directoryName)
{
	// Try to get the folder
	FolderObjectType* folder = GetFolderFromDirectory(_directoryPath);
	if (folder == nullptr)
	{
		return false;
	}

	// For each subfolder
	for (auto & folder : folder->subFolders)
	{
		// Compare the names
		if (folder->folderName.compare(_directoryName) == 0)
		{
			return true;
		}
	}

	return false;
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

std::vector<std::string> Packet::PacketObjectStructure::GetFolderList(std::vector<std::string>& _directoryPath)
{
	std::vector<std::string> result;

	// Try to get the folder
	FolderObjectType* folder = GetFolderFromDirectory(_directoryPath);
	if (folder == nullptr)
	{
		return result;

	}

	// For each child folder
	for (auto* childFolder : folder->subFolders)
	{
		result.push_back(childFolder->folderName);
	}

	// For each child file
	for (auto* childFile : folder->files)
	{
		result.push_back(childFile->fileName);
	}

	return result;
}

std::string Packet::PacketObjectStructure::GetRootName()
{
	return m_RootFolder->folderName;
}

Packet::PacketObjectStructure::FolderObjectType* Packet::PacketObjectStructure::GetFolderFromDirectory(std::vector<std::string>& _directoryPath, bool _returnParentFolder)
{
	// For each folder inside the path
	FolderObjectType* currentFolder = m_RootFolder;
	FolderObjectType* lastFolder = currentFolder;
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
				lastFolder = currentFolder;
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

	// Check if we should return the parent folder
	if (_returnParentFolder)
	{
		return lastFolder;
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

void Packet::PacketObjectStructure::LoadObjectStructureAux(FolderObjectType* _folder, std::vector<unsigned char>& _data, uint32_t& _location)
{
	// Save the folder name
	_folder->folderName = PacketFileDataOperations::ReadFromData(_data, _location);

	// Save the folder path
	_folder->folderPath = PacketFileDataOperations::ReadFromData(_data, _location);

	// Write the number of files
	uint32_t numberFiles = PacketFileDataOperations::ReadFromData<uint32_t>(_data, _location);

	// For each file
	for (unsigned int i=0; i<numberFiles; i++)
	{
		// The file
		FileObjectType* newFile = new FileObjectType();

		// Save the name
		newFile->fileName = PacketFileDataOperations::ReadFromData(_data, _location);

		// Save the extension
		newFile->fileExtension = PacketFileDataOperations::ReadFromData(_data, _location);

		// Save the file path
		newFile->filePath = PacketFileDataOperations::ReadFromData(_data, _location);

		// Save the hash identifier
		newFile->fileHashIdentifier = PacketFileDataOperations::ReadFromData<PacketObjectHashTable::PacketObjectHash>(_data, _location);

		// Insert the new file
		_folder->files.push_back(newFile);
	}

	// Write the number of sub folders
	uint32_t numberFolders = PacketFileDataOperations::ReadFromData<uint32_t>(_data, _location);

	// For each sub folder
	for (unsigned int i=0; i<numberFolders; i++)
	{
		// The folder
		FolderObjectType* newFolder = new FolderObjectType();

		// Call this method
		LoadObjectStructureAux(newFolder, _data, _location);

		// Insert the new subfolder
		_folder->subFolders.push_back(newFolder);
	}
}

void Packet::PacketObjectStructure::SaveObjectStructureAux(FolderObjectType* _folder, std::vector<unsigned char>& _data, uint32_t& _location)
{
	// Save the folder name
	PacketFileDataOperations::SaveToData(_data, _location, _folder->folderName);

	// Save the folder path
	PacketFileDataOperations::SaveToData(_data, _location, _folder->folderPath);

	// Write the number of files
	PacketFileDataOperations::SaveToData<uint32_t>(_data, _location, _folder->files.size());

	// For each file
	for (auto & file : _folder->files)
	{
		// Save the name
		PacketFileDataOperations::SaveToData(_data, _location, file->fileName);

		// Save the extension
		PacketFileDataOperations::SaveToData(_data, _location, file->fileExtension);

		// Save the file path
		PacketFileDataOperations::SaveToData(_data, _location, file->filePath);

		// Save the hash identifier
		PacketFileDataOperations::SaveToData<PacketObjectHashTable::PacketObjectHash>(_data, _location, file->fileHashIdentifier);
	}

	// Write the number of sub folders
	PacketFileDataOperations::SaveToData<uint32_t>(_data, _location, _folder->subFolders.size());

	// For each sub folder
	for (auto & folder : _folder->subFolders)
	{
		// Call this method
		 SaveObjectStructureAux(folder, _data, _location);
	}
}