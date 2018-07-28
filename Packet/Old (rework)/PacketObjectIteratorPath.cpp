////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketObjectIteratorPath.h"
#include "PacketStringOperations.h"

#include <experimental/filesystem>

Packet::PacketObjectIteratorPath::PacketObjectIteratorPath(PacketObjectStructure& _packetStructure) : m_PacketStructureReference(_packetStructure)
{
	// Set the initial data
	// ...
}

Packet::PacketObjectIteratorPath::~PacketObjectIteratorPath()
{
}

bool Packet::PacketObjectIteratorPath::ComposeSeek(std::string _path)
{
	// Check if the path is a folder or a seek command
	if (!PacketStringOperations::PathIsFolder(_path, false))
	{
		// Set the error
		return false;
	}

	// Compose the action directory
	std::vector<std::string> actionDirectory = ComposeActionDirectory(_path, true);

	// Check if the folder path if valid
	if (!m_PacketStructureReference.DirectoryFromPathIsValid(actionDirectory))
	{
		// Set the error
		return false;
	}

	// Set the new current directory
	m_CurrentDirectoryPath = actionDirectory;

	return true;
}

std::string Packet::PacketObjectIteratorPath::GetCurrentPath()
{
	return PacketStringOperations::ComposeDirectory(m_CurrentDirectoryPath);
}

std::vector<std::string>& Packet::PacketObjectIteratorPath::GetCurrentActionPath()
{
	return m_CurrentDirectoryPath;
}

void Packet::PacketObjectIteratorPath::SetCurrentPath(std::vector<std::string> _currentPath)
{
	m_CurrentDirectoryPath = _currentPath;
}

std::vector<std::string> Packet::PacketObjectIteratorPath::ComposeActionDirectory(std::string& _path, bool _seeking)
{
	// Split the path
	std::vector<std::string> splitPath = PacketStringOperations::SplitPath(_path);

	// Check if we have at last one string
	if (splitPath.size() != 0)
	{
		// Check the path is from the root element
		if (splitPath[0].compare(m_PacketStructureReference.GetRootName()) == 0)
		{
			// Remove the root directory
			splitPath.erase(splitPath.begin());

			// Its the path itself
			return splitPath;
		}
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