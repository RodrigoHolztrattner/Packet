////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketObjectTemporaryPath.h"
#include "PacketStringOperations.h"

#include <experimental/filesystem>

Packet::PacketObjectTemporaryPath::PacketObjectTemporaryPath(PacketObjectStructure& _packetStructure, PacketObjectIteratorPath& _packetIteratorPathReference) :
	m_PacketStructureReference(_packetStructure), 
	m_PacketIteratorPathReference(_packetIteratorPathReference)
{
	// Set the initial data
	// ...
}

Packet::PacketObjectTemporaryPath::~PacketObjectTemporaryPath()
{
}

std::string Packet::PacketObjectTemporaryPath::GetFullPath()
{
	return PacketStringOperations::ComposeDirectory(m_FullSplitPath, m_IsFile);
}

std::vector<std::string> Packet::PacketObjectTemporaryPath::GetFullSplitPath()
{
	return m_FullSplitPath;
}

std::string Packet::PacketObjectTemporaryPath::GetFolderPath()
{
	return PacketStringOperations::ComposeDirectory(m_FolderSplitPath, m_IsFile);
}

std::vector<std::string>& Packet::PacketObjectTemporaryPath::GetFolderSplitPath()
{
	return m_FolderSplitPath;
}

std::string Packet::PacketObjectTemporaryPath::GetFilename()
{
	return m_FileName;
}

std::string Packet::PacketObjectTemporaryPath::GetFileWithoutExtension()
{
	return m_FileNameNoExtension;
}

std::string Packet::PacketObjectTemporaryPath::GetFileExtension()
{
	return m_FileExtension;
}

bool Packet::PacketObjectTemporaryPath::IsFile()
{
	return m_IsFile;
}

bool Packet::PacketObjectTemporaryPath::IsFolder()
{
	return m_IsFolder;
}

bool Packet::PacketObjectTemporaryPath::IsValid()
{
	return m_IsValid;
}

bool Packet::PacketObjectTemporaryPath::ComposeTemporaryPath(std::string _appendPath)
{
	return ComposeTemporaryPathAux(PacketStringOperations::SplitPath(_appendPath));
}

bool Packet::PacketObjectTemporaryPath::ComposeTemporaryPath(std::vector<std::string> _appendPath)
{
	return ComposeTemporaryPathAux(_appendPath);
}

bool Packet::PacketObjectTemporaryPath::InsertExternalFilePath(std::string _externalFilePath)
{
	// Check if the _appendPath is a file
	if (!PacketStringOperations::PathIsFile(_externalFilePath))
	{
		return false;
	}

	// Split the external file path
	std::vector<std::string> splitExternalPath = PacketStringOperations::SplitPath(_externalFilePath);

	// Create a temporary split path
	std::vector<std::string> temporarySplitPath = splitExternalPath;

	// Remove the filename to get the file folder path
	if (temporarySplitPath.size() > 0)
	{
		// Remove the last element
		temporarySplitPath.erase(temporarySplitPath.begin() + temporarySplitPath.size() - 1);
	}

	// Set the file path / file name
	m_ExternalFilePath = _externalFilePath; // Use the full file path
	m_ExternalFolderPath = PacketStringOperations::ComposeDirectory(temporarySplitPath); // Use our path without the file name
	m_ExternalFileName = splitExternalPath[splitExternalPath.size()-1]; // Use the last element (it should be the file name)

	return true;
}

bool Packet::PacketObjectTemporaryPath::MergeExternalFileName()
{
	// Check if the current operation mode is the folder one
	if (!m_IsFolder || !m_IsValid)
	{
		// We cant operate on files / invalid data
		return false;
	}

	// Get the current full split folder path
	std::vector<std::string> currentSplitFolderPath = GetFolderSplitPath();

	// Add the filename to the end
	currentSplitFolderPath.push_back(GetExternalFilename());

	// Compose again the temporary path
	return ComposeTemporaryPath(currentSplitFolderPath);
}

std::string Packet::PacketObjectTemporaryPath::GetExternalFilePath()
{
	return m_ExternalFilePath;
}

std::string Packet::PacketObjectTemporaryPath::GetExternalFolderPath()
{
	return m_ExternalFolderPath;
}

std::string Packet::PacketObjectTemporaryPath::GetExternalFilename()
{
	return m_ExternalFileName;
}

/////////////
// PRIVATE //
/////////////

bool Packet::PacketObjectTemporaryPath::ComposeTemporaryPathAux(std::vector<std::string> _appendPath)
{
	// Check if the _appendPath is a file
	if (PacketStringOperations::PathIsFile(_appendPath))
	{
		// Set that we are a file
		m_IsFile = true;
		m_IsFolder = false;

		// Create a temporary string path
		std::string temporaryStringPath = PacketStringOperations::ComposeDirectory(_appendPath, m_IsFile);

		/*
		// Create a temporary split path
		std::vector<std::string> temporarySplitPath = _appendPath;
		
		// If the size is bigger then 1, remove the last element (to get the folder path)
		if (temporarySplitPath.size() > 1)
		{
			// Remove the last element (the file name)
			temporarySplitPath.erase(temporarySplitPath.begin() + temporarySplitPath.size() - 1);
		}

		// Create the string folder path from the split one
		std::string folderPath = PacketStringOperations::ComposeDirectory(temporarySplitPath);
		*/

		// Set the file string values
		m_FileName = _appendPath[_appendPath.size() - 1]; // Use the last element (it should be the file name)
		m_FileNameNoExtension = std::experimental::filesystem::path(temporaryStringPath).stem().string();
		m_FileExtension = std::experimental::filesystem::path(temporaryStringPath).extension().string();
	}
	// Check if the _appendPath is a folder
	else if (PacketStringOperations::PathIsFolder(_appendPath, true, true))
	{
		// Set that we are a folder
		m_IsFile = false;
		m_IsFolder = true;
	}
	// Set invalid
	else
	{
		// Set the boolean variables
		m_IsFile = false;
		m_IsFolder = false;
		m_IsValid = false;
		return false;
	}

	// Try to join the input path with the current one
	if (!TryJoinWithCurrentPath(_appendPath))
	{
		// Ok, we should try to use the input path as an absolute one
		if (!TryUseAbsolutePath(_appendPath))
		{
			// Ops, we have a problem, invalid input path
			m_IsValid = false;
			return false;
		}
	}

	// Ok we found a valid path, determine the full path excluding any file name if there is one
	m_FolderSplitPath = m_FullSplitPath;
	if (m_IsFile && m_FolderSplitPath.size() > 0)
	{
		// Remove the file name from the folder split path
		m_FolderSplitPath.erase(m_FolderSplitPath.begin() + m_FolderSplitPath.size() - 1);
	}

	// Set is valid
	m_IsValid = true;

	return true;
}

bool Packet::PacketObjectTemporaryPath::TryJoinWithCurrentPath(std::vector<std::string> _appendPath)
{
	// Get the current path
	std::vector<std::string> currentPath = m_PacketIteratorPathReference.GetCurrentActionPath();

	// Determine the append path size (subtract one if it is a file)
	uint32_t appendPathSize = m_IsFile ? _appendPath.size() - 1 : _appendPath.size();

	// For each folder inside the append path
	for (unsigned int i = 0; i < appendPathSize; i++)
	{
		// Insert the append path into the current path
		currentPath.push_back(_appendPath[i]);
	}

	// Check if the given path is valid inside our packet structure
	if (!m_PacketStructureReference.DirectoryFromPathIsValid(currentPath))
	{
		return false;
	}

	// Ok we found a valid path, check if we should insert a filename at the end
	if (m_IsFile)
	{
		// Insert the last string from the append path (it should be a filename + extension)
		currentPath.push_back(_appendPath[_appendPath.size()-1]);
	}

	// Set the full split path
	m_FullSplitPath = currentPath;

	return true;
}

bool Packet::PacketObjectTemporaryPath::TryUseAbsolutePath(std::vector<std::string> _appendPath)
{
	// Set the current path
	std::vector<std::string> currentPath = _appendPath;

	// If the input path is a file, remove the file name from the current path
	if (m_IsFile && _appendPath.size() > 0)
	{
		// Remove the last string (file name)
		currentPath.erase(currentPath.begin() + currentPath.size() - 1);
	}

	// Check if the current path is valid
	if (!m_PacketStructureReference.DirectoryFromPathIsValid(currentPath))
	{
		return false;
	}

	// Set the full split path (use the append path as it has a possible file name if there is one)
	m_FullSplitPath = _appendPath;

	return true;
}