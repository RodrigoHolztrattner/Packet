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
	// Get the directory path
	std::vector<std::string> directoryPath = PacketStringOperations::SplitPath(_path);

	// Compose the new directory
	std::vector<std::string> newDirectory = m_CurrentDirectoryPath;
	for (auto& folder : directoryPath)
	{
		newDirectory.push_back(folder);
	}

	// Check if the folder path if valid
	if (!m_PacketStructureReference.DirectoryFromPathIsValid(newDirectory))
	{
		return false;
	}

	// Set the new current directory
	m_CurrentDirectoryPath = newDirectory;

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

bool Packet::PacketObjectIterator::Put(std::string _filePath, std::string _iFileLocation)
{
	// Get the file name from the path
	std::string fileName = PacketStringOperations::GetFilenameFromPath(_filePath);

	// Get the directory path
	std::vector<std::string> directoryPath = PacketStringOperations::SplitPath(_iFileLocation);

	return PutAux(_filePath, fileName, directoryPath, _iFileLocation);
}

bool Packet::PacketObjectIterator::Get(std::string _oFileLocation)
{

	return true;
}

bool Packet::PacketObjectIterator::Get(std::string _filePath, std::string _oFileLocation)
{
	return true;
}

bool Packet::PacketObjectIterator::Delete(std::string _path)
{
	return true;
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