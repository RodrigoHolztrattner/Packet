////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketObjectStructure.h"

Packet::PacketObjectStructure::PacketObjectStructure(std::string _packetName)
{
	// Set the initial data
	m_PacketObjectName = _packetName;

	// Try to load the object structure
	if (!LoadObjectStructure())
	{
		// Create the default root folder
		m_RootFolder = new FolderObjectType;
		m_RootFolder->folderName = _packetName;
	}
}

Packet::PacketObjectStructure::~PacketObjectStructure()
{
	// Try to save the object structure
	if (!SaveObjectStructure())
	{
		
	}
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



bool Packet::PacketObjectStructure::LoadObjectStructure()
{
	// Compose the structure name
	std::string structureName = m_PacketObjectName + StructureExtensionType;

	// Open the file
	std::ifstream file;
	file.open(structureName, std::ios::in | std::ios::binary);
	
	// Check if we are ok
	if (!file.good())
	{
		return false;
	}
	
	// Create the root folder
	m_RootFolder = new FolderObjectType;

	// Load the object structure
	if (!LoadObjectStructureAux(m_RootFolder, file))
	{
		return false;
	}

	return true;
}

bool Packet::PacketObjectStructure::LoadObjectStructureAux(FolderObjectType* _folder, std::ifstream& _ifstream)
{
	char name[64];

	// Copy the folder name and load it
	_ifstream.read(name, sizeof(char) * 64);
	_folder->folderName = std::string(name);
	
	// Copy the folder path and load it
	_ifstream.read(name, sizeof(char) * 64);
	_folder->folderPath = std::string(name);

	// Read the number of files
	unsigned int numberFiles;
	_ifstream.read((char*)&numberFiles, sizeof(unsigned int));

	// For each file
	for (unsigned int i=0; i<numberFiles; i++)
	{
		// Create the new file
		FileObjectType* newFile = new FileObjectType();

		// Copy the name and load it
		_ifstream.read(name, sizeof(char) * 64);
		newFile->fileName = std::string(name);

		// Copy the file path and load it
		_ifstream.read(name, sizeof(char) * 64);
		newFile->filePath = std::string(name);

		// Load the identifier
		_ifstream.read((char*)&newFile->fileHashIdentifier, sizeof(unsigned int));

		// Insert the new folder
		_folder->files.push_back(newFile);
	}

	// Read the number of sub folders
	unsigned int numberFolders;
	_ifstream.read((char*)&numberFolders, sizeof(unsigned int));

	// For each sub folder
	for (unsigned int i=0; i<numberFolders; i++)
	{
		// Create a new folder
		FolderObjectType* newFolder = new FolderObjectType();

		// Call this method
		bool result = LoadObjectStructureAux(newFolder, _ifstream);
		if (!result)
		{
			return false;
		}

		// Insert the new folder
		_folder->subFolders.push_back(newFolder);
	}

	return true;
}

bool Packet::PacketObjectStructure::SaveObjectStructure()
{
	// Compose the structure name
	std::string structureName = m_PacketObjectName + StructureExtensionType;

	// Open the file
	std::ofstream file;
	file.open(structureName, std::ios::out | std::ios::binary);

	// Check if we are ok
	if (!file.good())
	{
		return false;
	}

	// Save the object structure
	if (!SaveObjectStructureAux(m_RootFolder, file))
	{
		return false;
	}

	return true;
}

bool Packet::PacketObjectStructure::SaveObjectStructureAux(FolderObjectType* _folder, std::ofstream& _ofstream)
{
	char name[64];

	// Copy the folder name and save it
	strcpy_s(name, _folder->folderName.c_str());
	_ofstream.write(name, sizeof(char) * 64);

	// Copy the folder path and save it
	strcpy_s(name, _folder->folderPath.c_str());
	_ofstream.write(name, sizeof(char) * 64);

	// Write the number of files
	size_t numberFiles = _folder->files.size();
	_ofstream.write((char*)&numberFiles, sizeof(unsigned int));

	// For each file
	for (auto & file : _folder->files)
	{
		// Copy the name and save it
		strcpy_s(name, file->fileName.c_str());
		_ofstream.write(name, sizeof(char) * 64);

		// Copy the file path and save it
		strcpy_s(name, file->filePath.c_str());
		_ofstream.write(name, sizeof(char) * 64);

		// Save the identifier
		size_t identifier = file->fileHashIdentifier;
		_ofstream.write((char*)&identifier, sizeof(unsigned int));
	}

	// Write the number of sub folders
	size_t numberFolders = _folder->subFolders.size();
	_ofstream.write((char*)&numberFolders, sizeof(unsigned int));

	// For each sub folder
	for (auto & folder : _folder->subFolders)
	{
		// Call this method
		bool result = SaveObjectStructureAux(folder, _ofstream);
		if (!result)
		{
			return false;
		}
	}

	return true;
}