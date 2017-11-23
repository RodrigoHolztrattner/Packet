////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketManager.h"

Packet::PacketManager::PacketManager()
{
	// Set the initial data
	m_RootFolder.folderId = RootDirectory;
	m_RootFolder.folderName.SetString("root");
	m_RootFolder.childFileInfos.reserve(1000); //TODO: Arrumar esse fix, quando um rezize é feito perdemos as referências
	m_RootFolder.childFolders.reserve(100);
}

Packet::PacketManager::~PacketManager()
{
}

static const char temp[] = "folder1/folder2/folder3/file1.txt";

/*
	- Adicionar um modo de exeução no manager que permite usar tudo ou apenas o vault (apenas o vault = sem texto e apenas index)

*/

bool Packet::PacketManager::Initialize(const char* _rootFileName)
{
	// Initialize the index loader
	if (!m_IndexLoader.Initialize(_rootFileName, &m_RootFolder, m_IndexData))
	{
		return false;
	}
	
	// Change the loader operation mode
	ChangeLoaderOperationMode(PacketMode::Editor);

	return true;
}

void Packet::PacketManager::Release()
{

}

Packet::PacketDirectory* Packet::PacketManager::SeekDir(const char* _dirPath)
{
	// Find the folder using the dir path
	PacketDirectory* dirReference = FindFolderByPath(&m_RootFolder, _dirPath, strlen(_dirPath));
	if (dirReference != nullptr)
	{
		return dirReference;
	}

	return nullptr;
}

Packet::PacketDirectory* Packet::PacketManager::SeekDir(uint32_t _dirIdentifier)
{
	// Root dir?
	if (_dirIdentifier == RootDirectory)
	{
		return &m_RootFolder;
	}

	// Try to find the folder using the id
	if (m_IndexData.folderIdentifierReferences.find(_dirIdentifier) != m_IndexData.folderIdentifierReferences.end())
	{
		return m_IndexData.folderIdentifierReferences[_dirIdentifier];
	}
	
	return nullptr;
}

Packet::PacketFile* Packet::PacketManager::FindFile(const char* _fileName, PacketDirectory* _currentDir, bool _recursive)
{
	// Check if we should use the root dir
	if (_currentDir == nullptr)
	{
		// Use the root dir
		_currentDir = &m_RootFolder;
	}

	// Find the filename from the path
	uint32_t fileStrinSize;
	const char* fileName = FindFileNameInFormatedPath(_fileName, fileStrinSize);
	if (fileName == nullptr)
	{
		return nullptr;
	}

	// Find the folder using the path
	PacketDirectory* directory = FindFolderByPath(_currentDir, _fileName, strlen(_fileName) - fileStrinSize);
	if (directory == nullptr)
	{
		return nullptr;
	}

	// Find the folder using the dir
	PacketFile* fileInfoReference = FindFileInfoInside(directory, fileName, strlen(fileName), _recursive);
	if (fileInfoReference != nullptr)
	{
		return fileInfoReference;
	}

	return nullptr;
}

Packet::PacketFile* Packet::PacketManager::FindFile(const char* _fileName, bool _recursive)
{
	return FindFile(_fileName, &m_RootFolder, _recursive);
}

Packet::PacketFile* Packet::PacketManager::FindFile(uint32_t _fileIdentifier)
{
	// Try to find the file info using the id
	if (m_IndexData.fileIdentifierReferences.find(_fileIdentifier) != m_IndexData.fileIdentifierReferences.end())
	{
		return m_IndexData.fileIdentifierReferences[_fileIdentifier];
	}

	return nullptr;
}

Packet::PacketDirectory* Packet::PacketManager::CreateDir(const char* _dirName, PacketDirectory* _currentDir)
{
	// Check if we should use the root dir
	if (_currentDir == nullptr)
	{
		// Use the root dir
		_currentDir = &m_RootFolder;
	}

	return FindFolderByPath(_currentDir, _dirName, strlen(_dirName), true);
}

Packet::PacketFile* Packet::PacketManager::CreateFile(const char* _filePath, const char* _externalRawPath, PacketDirectory* _currentDir)
{
	// Check if we should use the root dir
	if (_currentDir == nullptr)
	{
		// Use the root dir
		_currentDir = &m_RootFolder;
	}

	// Find the filename from the given path
	uint32_t fileNameSize;
	const char* filename = FindFileNameInFormatedPath(_filePath, fileNameSize);
	if (filename == nullptr)
	{
		return nullptr;
	}

	// Find the folder using the path
	PacketDirectory* fileFolder = FindFolderByPath(_currentDir, _filePath, strlen(_filePath) - fileNameSize, true);
	if (fileFolder == nullptr)
	{
		return nullptr;
	}

	// Create the new file
	PacketFile* newFile = CreateFileAux(fileFolder, filename, _externalRawPath, fileNameSize);
	if (newFile == nullptr)
	{
		return nullptr;
	}

	return newFile;
}

Packet::PacketDirectory* Packet::PacketManager::FindFolderByPath(PacketDirectory* _fromFolder, const char* _dirPath, uint32_t _dirPathSize, bool _createIfDontExist)
{
	// Set the initial current folder
	PacketDirectory* currentNode = _fromFolder;

	// Get the total dir path size
	uint32_t totalPathSize = _dirPathSize;

	// Set the current search location inside the string
	uint32_t stringSearchLocation = 0;

	// The next string location
	uint32_t nextStringLocation = 0;

	// Go throught each folder
	while (true)
	{
		// The string size
		uint32_t stringSize;
		
		// Get the next folder name
		const char* folderName = FindNameInFormatedPath(_dirPath, stringSearchLocation, stringSize, nextStringLocation, totalPathSize);
		if (folderName == nullptr)
		{
			// No more folder to look
			break;
		}

		// Find the folder
		PacketDirectory* folder = FindFolderInside(currentNode, folderName, stringSize, false);
		if (folder == nullptr)
		{
			// Check if we should create a new folder
			if (_createIfDontExist)
			{
				// Try to create the folder
				PacketDirectory* createdFolder = CreateFolderAux(currentNode, folderName, stringSize);
				if (createdFolder == nullptr)
				{
					// Problem creating the new folder (invalid name)
					return nullptr;
				}

				// Try again (we should be able tu use it)
				continue;
			}

			// We cant find the folder (the folder really exists?)
			return nullptr;
		}

		// Set the new current folder
		currentNode = folder;

		// Atualize the path
		_dirPath = &_dirPath[nextStringLocation];

		// Atualize the path size
		totalPathSize -= nextStringLocation;
	}

	return currentNode;
}

Packet::PacketDirectory* Packet::PacketManager::FindFolderInside(PacketDirectory* _fromFolder, const char* _folderName, uint32_t _folderNameSize, bool _recursive)
{
	// For each child folder
	for (auto& childFolder : _fromFolder->childFolders)
	{
		// Compare the names
		if (childFolder.folderName.IsEqual(_folderName, _folderNameSize))
		{
			return &childFolder;
		}

		// If recursive
		if (_recursive)
		{
			// Check inside the child folder
			PacketDirectory* childResult = FindFolderInside(&childFolder, _folderName, _folderNameSize, _recursive);
			if (childResult != nullptr)
			{
				return childResult;
			}
		}
	}

	return nullptr;
}

Packet::PacketFile* Packet::PacketManager::FindFileInfoInside(PacketDirectory* _fromFolder, const char* _fileName, uint32_t _fileNameSize, bool _recursive)
{
	// For each child file info
	for (auto& childFileInfo : _fromFolder->childFileInfos)
	{
		// Compare the names
		if (childFileInfo.fileName.IsEqual(_fileName, _fileNameSize))
		{
			return &childFileInfo;
		}
	}

	// If recursive
	if (_recursive)
	{
		// For each child folder
		for (auto& childFolder : _fromFolder->childFolders)
		{
			// Check inside the child folder
			PacketFile* childResult = FindFileInfoInside(&childFolder, _fileName, _fileNameSize, _recursive);
			if (childResult != nullptr)
			{
				return childResult;
			}

		}
	}

	return nullptr;
}

const char* Packet::PacketManager::FindNameInFormatedPath(const char* _formatedPath, uint32_t& _currentPosition, uint32_t& _stringSize, uint32_t& _nextStringPosition, uint32_t _maxSize)
{
	// Zero the string size
	_stringSize = 0;

	// Check if the current position is valid
	if (_currentPosition >= _maxSize || _formatedPath[_currentPosition] == 0)
	{
		// No more formated name
		return nullptr;
	}

	// Until we dont overlap
	uint32_t localSearchPosition = _currentPosition;
	while (localSearchPosition < _maxSize)
	{
		// Check if we found the separator
		if (_formatedPath[localSearchPosition] == Separator)
		{
			// Set the next screen position
			_nextStringPosition = localSearchPosition + 1;

			break;
		}

		// Increment the string size and the search location
		_stringSize++;
		localSearchPosition++;
		_nextStringPosition = localSearchPosition;
	}

	// Ok, there is our string
	return &_formatedPath[_currentPosition];
}

const char* Packet::PacketManager::FindFileNameInFormatedPath(const char* _formatedPath, uint32_t& stringSize)
{
	// Zero the string size
	stringSize = 0;

	// Set the path size
	uint32_t pathSize = strlen(_formatedPath);
	if (!pathSize)
	{
		return nullptr;
	}

	while (stringSize < pathSize)
	{
		// Check if we found a separator
		if (_formatedPath[pathSize - stringSize] == Separator)
		{
			// Decrement 1 from the string size (jump the separator)
			stringSize--;

			break;
		}

		// Increment the string size
		stringSize++;
	}

	// Check the string size
	if (!stringSize)
	{
		return nullptr;
	}

	return &_formatedPath[pathSize - stringSize];
}

Packet::PacketDirectory* Packet::PacketManager::CreateFolderAux(PacketDirectory* _fromFolder, const char* _withName, uint32_t _nameSize)
{
	// Check if the folder already have a child with the given name
	for (auto& childFolder : _fromFolder->childFolders)
	{
		// Compare the names
		if (childFolder.folderName.IsEqual(_withName, _nameSize))
		{
			return nullptr;
		}
	}

	// Ok we are free to create a new one
	PacketDirectory newFolder = {};
	newFolder.folderName.SetString(_withName, _nameSize);
	newFolder.folderId = GetValidFolderIdentifier();

	newFolder.childFileInfos.reserve(100); //TODO: Fix temporário, acontece que quando adicionamos algo a esses vetores e eles crescem, perdemos as referências dos objetos
	newFolder.childFolders.reserve(100);

	// Insert into the child list
	uint32_t insertIndex = _fromFolder->childFolders.size();
	_fromFolder->childFolders.push_back(newFolder);
	
	// Insert into the map
	m_IndexData.folderIdentifierReferences[newFolder.folderId] = &_fromFolder->childFolders[insertIndex];

	// Update all map data
	for (auto& packetFolder : _fromFolder->childFolders)
	{
		m_IndexData.folderIdentifierReferences[packetFolder.folderId] = &packetFolder; //TODO: Pensar em uma forma melhor de fazer isso (ele perde a referencia porque ocorre resize acima as vezes (_fromFolder->childFileInfos.push_back(newFile);)
	}

	return &_fromFolder->childFolders[insertIndex];
}

Packet::PacketFile* Packet::PacketManager::CreateFileAux(PacketDirectory* _fromFolder, const char* _withName, const char* _withExternalRawPath, uint32_t _nameSize)
{
	// Check if the folder already have a child with the given name
	for (auto& childFile : _fromFolder->childFileInfos)
	{
		// Compare the names
		if (childFile.fileName.IsEqual(_withName, _nameSize))
		{
			return nullptr;
		}
	}

	// Ok we are free to create a new one
	PacketFile newFile = {};
	newFile.fileName.SetString(_withName, _nameSize);
	newFile.fileExternalPath.SetString(_withExternalRawPath, strlen(_withExternalRawPath));
	newFile.fileId = GetValidFileIdentifier();

	// Insert into the child list
	uint32_t insertIndex = _fromFolder->childFileInfos.size();
	_fromFolder->childFileInfos.push_back(newFile);

	// Insert into the map
	m_IndexData.fileIdentifierReferences[newFile.fileId] = &_fromFolder->childFileInfos[insertIndex];

	// Update all map data
	for (auto& packetFile : _fromFolder->childFileInfos)
	{
		m_IndexData.fileIdentifierReferences[packetFile.fileId] = &packetFile; //TODO: Pensar em uma forma melhor de fazer isso (ele perde a referencia porque ocorre resize acima as vezes (_fromFolder->childFileInfos.push_back(newFile);)
	}

	return &_fromFolder->childFileInfos[insertIndex];
}

uint32_t Packet::PacketManager::GetValidFolderIdentifier()
{
	// Check if we have any free ids
	if (m_IndexData.folderIdentifierFreelist.size())
	{
		// Get the last valid id
		uint32_t validId = m_IndexData.folderIdentifierFreelist[m_IndexData.folderIdentifierFreelist.size() - 1];

		// Erase the last element
		m_IndexData.folderIdentifierFreelist.erase(m_IndexData.folderIdentifierFreelist.begin() + m_IndexData.folderIdentifierFreelist.size() - 1);

		return validId;
	}

	// Return a new valid identifier
	return m_IndexData.folderIdentifierReferences.size();
}

uint32_t Packet::PacketManager::GetValidFileIdentifier()
{
	// Check if we have any free ids
	if (m_IndexData.fileIdentifierFreelist.size())
	{
		// Get the last valid id
		uint32_t validId = m_IndexData.fileIdentifierFreelist[m_IndexData.fileIdentifierFreelist.size() - 1];

		// Erase the last element
		m_IndexData.fileIdentifierFreelist.erase(m_IndexData.fileIdentifierFreelist.begin() + m_IndexData.fileIdentifierFreelist.size() - 1);

		return validId;
	}

	// Return a new valid identifier
	return m_IndexData.fileIdentifierReferences.size();
}

void Packet::PacketManager::FreeFolderIdentifier(uint32_t _identifier)
{
	// Insert into the freelist
	m_IndexData.folderIdentifierFreelist.push_back(_identifier);
}

void Packet::PacketManager::FreeFileIdentifier(uint32_t _identifier)
{
	// Insert into the freelist
	m_IndexData.fileIdentifierFreelist.push_back(_identifier);
}