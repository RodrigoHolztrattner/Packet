////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketObjectIterator.h"
#include "PacketStringOperations.h"

#include <experimental/filesystem>

Packet::PacketObjectIterator::PacketObjectIterator(PacketObjectManager& _packetManagerReference, PacketObjectStructure& _packetStructureManager, PacketObjectHashTable& _packetHashTableReference) : 
	m_PacketManagerReference(_packetManagerReference),
	m_PacketStructureReference(_packetStructureManager), 
	m_PacketHashTableReference(_packetHashTableReference)
{
	// Set the initial data
	// ...
}

Packet::PacketObjectIterator::~PacketObjectIterator()
{
}

bool Packet::PacketObjectIterator::Seek(std::string _path)
{
	// Check if the path is a folder or a seek command
	if (!PacketStringOperations::PathIsFolder(_path, false))
	{
		return false;
	}

	// Compose the action directory
	std::vector<std::string> actionDirectory = ComposeActionDirectory(_path, true);

	// Check if the folder path if valid
	if (!m_PacketStructureReference.DirectoryFromPathIsValid(actionDirectory))
	{
		return false;
	}

	// Set the new current directory
	m_CurrentDirectoryPath = actionDirectory;

	return true;
}

bool Packet::PacketObjectIterator::Put(std::string _filePath)
{
	// Get the file name from the path
	std::string fileName = PacketStringOperations::GetFilenameFromPath(_filePath);

	// Compose the string directory
	std::string stringDir = PacketStringOperations::ComposeDirectory(m_CurrentDirectoryPath);

	return PutAux(_filePath, fileName, m_CurrentDirectoryPath, stringDir);
}

bool Packet::PacketObjectIterator::Put(std::string _filePath, std::string iFolderLocation)
{
	// Check if the iFolderLocation is a folder
	if (!PacketStringOperations::PathIsFolder(iFolderLocation))
	{
		return false;
	}

	// Get the file name from the path
	std::string fileName = PacketStringOperations::GetFilenameFromPath(_filePath);

	// Get the directory path
	std::vector<std::string> directoryPath = PacketStringOperations::SplitPath(iFolderLocation);

	return PutAux(_filePath, fileName, directoryPath, iFolderLocation);
}

bool Packet::PacketObjectIterator::MakeDir(std::string _dirPath)
{
	// Check if the dir is a folder path
	if (!PacketStringOperations::PathIsFolder(_dirPath))
	{
		return false;
	}

	// Break the dir path
	std::vector<std::string> dirPathVec = PacketStringOperations::SplitPath(_dirPath);

	// If we have at last one element
	if (dirPathVec.size() > 0)
	{
		// Look for the root folder
		if (dirPathVec[0].compare(m_PacketStructureReference.GetRootName()) == 0)
		{
			// Remove the root folder from the vector
			dirPathVec.erase(dirPathVec.begin() + 0);
		}
	}

	// Compose the action directory
	std::vector<std::string> currentDirectory = ComposeActionDirectory(_dirPath);

	// For each folder
	for (auto & folder : dirPathVec)
	{
		// Check if we already have a file on that location
		if (!m_PacketStructureReference.DirectoryFromPathIsValid(currentDirectory, folder))
		{
			// try to create the new folder
			if (!m_PacketStructureReference.InsertFolder(folder, currentDirectory))
			{
				return false;
			}
		}

		// Move to the created directory
		currentDirectory.push_back(folder);
	}

	return true;
}

bool Packet::PacketObjectIterator::Get(std::string _oFileLocation)
{

	return true;
}

bool Packet::PacketObjectIterator::Get(std::string _filePath, std::string _oFileLocation)
{
	// Check if the _filePath is a file path
	if (!PacketStringOperations::PathIsFile(_filePath))
	{
		return false;
	}

	return true;
}

bool Packet::PacketObjectIterator::Delete(std::string _path)
{
	return true;
}

std::vector<std::string> Packet::PacketObjectIterator::List()
{
	return m_PacketStructureReference.GetFolderList(m_CurrentDirectoryPath);
}

std::vector<std::string> Packet::PacketObjectIterator::List(std::string _path)
{
	// Split the path
	std::vector<std::string> splitPath = PacketStringOperations::SplitPath(_path);

	// If we have at last one element
	if (splitPath.size() > 0)
	{
		// Look for the root folder
		if (splitPath[0].compare(m_PacketStructureReference.GetRootName()) == 0)
		{
			// Remove the root folder from the vector
			splitPath.erase(splitPath.begin() + 0);
		}
	}

	return m_PacketStructureReference.GetFolderList(splitPath);
}

std::string Packet::PacketObjectIterator::GetCurrentPath()
{
	std::string result = m_PacketStructureReference.GetRootName() + ":\\";
	
	// For each folder
	for (int i=0; i<m_CurrentDirectoryPath.size(); i++)
	{
		// Add it to the result
		result += m_CurrentDirectoryPath[i];
		if (i != m_CurrentDirectoryPath.size() - 1) result += '\\';
	}
	
	return result;
}

bool Packet::PacketObjectIterator::PutAux(std::string _filePath, std::string _fileName, std::vector<std::string> _dir, std::string _stringDir)
{
	/*
	// Check if the given path is valid
	if (!std::experimental::filesystem::is_regular_file(_filePath))
	{
		return false;
	}
	*/

	// Check if we already have a file on that location
	if (m_PacketStructureReference.FileFromPathIsValid(_dir, _fileName))
	{
		return false;
	}

	// Insert the new file inside the object data
	PacketObjectManager::FileFragmentIdentifier fileFragmentIdentifier;
	if (!m_PacketManagerReference.InsertFile(_filePath, fileFragmentIdentifier))
	{
		return false;
	}

	// Insert into the hash table
	uint32_t fileHashIdentifier = m_PacketHashTableReference.InsertEntry(_stringDir, fileFragmentIdentifier);

	// Insert the new file inside the current folder
	if (!m_PacketStructureReference.InsertFile(_fileName, fileHashIdentifier, _dir))
	{
		return false;
	}

	return true;
}

std::vector<std::string> Packet::PacketObjectIterator::ComposeActionDirectory(std::string& _path, bool _seeking)
{
	// Split the path
	std::vector<std::string> splitPath = PacketStringOperations::SplitPath(_path);

	// Check the path is from the root element
	if (splitPath[0].compare(m_PacketStructureReference.GetRootName()) == 0)
	{
		// Its the path itself
		return{};
	}

	// If we are seeking
	if (_seeking)
	{
		// Join the entry path with the current directory
		return PacketStringOperations::JoinDirectorySeek(m_CurrentDirectoryPath, splitPath);
	}

	// Ok, our current directory
	return m_CurrentDirectoryPath;
}