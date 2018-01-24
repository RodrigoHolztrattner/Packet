////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketObjectIterator.h"
#include "PacketStringOperations.h"

#include <experimental/filesystem>

Packet::PacketObjectIterator::PacketObjectIterator(PacketObjectManager& _packetManagerReference, PacketObjectStructure& _packetStructureManager, PacketObjectHashTable& _packetHashTableReference) : 
	m_PacketManagerReference(_packetManagerReference),
	m_PacketStructureReference(_packetStructureManager), 
	m_PacketHashTableReference(_packetHashTableReference),
	m_IteratorPath(_packetStructureManager)
{
	// Set the initial data
	// ...
}

Packet::PacketObjectIterator::~PacketObjectIterator()
{
}

bool Packet::PacketObjectIterator::Seek(std::string _path)
{
	// Compose the seek path
	if (!m_IteratorPath.ComposeSeek(_path))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorInvalidDirectory);
		return false;
	}

	return true;
}

bool Packet::PacketObjectIterator::Put(unsigned char* _data, uint32_t _size)
{
	return Put(_data, _size, m_IteratorPath.GetCurrentPath());
}

bool Packet::PacketObjectIterator::Put(unsigned char* _data, uint32_t _size, std::string iFolderLocation)
{
	// Compose the temporary path
	PacketObjectTemporaryPath temporaryPath(m_PacketStructureReference, m_IteratorPath);
	if (!temporaryPath.ComposeTemporaryPath(iFolderLocation))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorPathNotDirectory);
		return false;
	}
	
	// Check if we can add this
	if (m_PacketHashTableReference.EntryExist(temporaryPath.GetFullPath()))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorHashDuplicate);
		return false;
	}

	// Check if we already have a file on that location
	if (m_PacketStructureReference.FileFromPathIsValid(temporaryPath.GetFolderSplitPath(), temporaryPath.GetFilename()))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorFileFromPathInvalid);
		return false;
	}

	// Insert the data inside the object
	PacketObjectManager::FileFragmentIdentifier fileFragmentIdentifier;
	if (!m_PacketManagerReference.InsertData(_data, _size, fileFragmentIdentifier))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorInvalidFileData);
		return false;
	}

	// Insert into the hash table
	uint64_t fileHashIdentifier = m_PacketHashTableReference.InsertEntry(temporaryPath.GetFullPath(), fileFragmentIdentifier, fileHashIdentifier);

	// Insert the new file inside the current folder
	if (!m_PacketStructureReference.InsertFile(temporaryPath.GetFilename(), fileHashIdentifier, temporaryPath.GetFolderSplitPath()))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorStructureInsert);
		return false;
	}

	return true;
}

bool Packet::PacketObjectIterator::Put(std::string _externalFilePath)
{
	return Put(_externalFilePath, m_IteratorPath.GetCurrentPath());
}

bool Packet::PacketObjectIterator::Put(std::string _externalFilePath, std::string iFolderLocation)
{
	// Compose the temporary path
	PacketObjectTemporaryPath temporaryPath(m_PacketStructureReference, m_IteratorPath);
	if (!temporaryPath.ComposeTemporaryPath(iFolderLocation))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorPathNotDirectory);
		return false;
	}

	// Put the external file path
	if (!temporaryPath.InsertExternalFilePath(_externalFilePath))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorInvalidFile);
		return false;
	}

	// Merge the output file name with the current temporary path
	if (!temporaryPath.MergeExternalFileName())
	{
		return false;
	}
	
	// _externalFilePath, m_IteratorPath.GetTemporaryPathFilename(), m_IteratorPath.GetTemporaryPathActionDirectory(), m_IteratorPath.GetTemporaryPathStringDirectory()
	return PutAux(temporaryPath);
}

bool Packet::PacketObjectIterator::Get(std::string _iFileLocation, unsigned char* _data, uint32_t _size)
{
	// Compose the temporary path
	PacketObjectTemporaryPath temporaryPath(m_PacketStructureReference, m_IteratorPath);
	if (!temporaryPath.ComposeTemporaryPath(_iFileLocation))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorPathNotFile);
		return false;
	}

	// Check if we have a file on that location
	if (!m_PacketStructureReference.FileFromPathIsValid(temporaryPath.GetFolderSplitPath(), temporaryPath.GetFilename()))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorFileFromPathDuplicated);
		return false;
	}

	// Get the file fragment identifier and check if it is valid
	PacketObjectManager::FileFragmentIdentifier* fileFragmentIdentifier = m_PacketHashTableReference.GetEntry(temporaryPath.GetFullPath());
	if (fileFragmentIdentifier == nullptr)
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorHashDuplicate);
		return false;
	}

	// Get the file
	if (!m_PacketManagerReference.GetData(_data, *fileFragmentIdentifier))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorRetrieveData);
		return false;
	}

	return true;
}

bool Packet::PacketObjectIterator::Get(std::string _iFileLocation)
{
	// Check if the iFileLocation is a file
	if (!PacketStringOperations::PathIsFile(_iFileLocation))
	{
		m_ErrorObject.Set(PacketErrorPathNotFile);
		return false;
	}

	// Get the file name from the path
	std::string fileName = PacketStringOperations::GetFilenameFromPath(_iFileLocation);

	return Get(_iFileLocation, fileName);
}

bool Packet::PacketObjectIterator::Get(std::string _iFileLocation, std::string _oFileLocation)
{
	// Compose the temporary path
	PacketObjectTemporaryPath temporaryPath(m_PacketStructureReference, m_IteratorPath);
	if (!temporaryPath.ComposeTemporaryPath(_iFileLocation))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorPathNotFile);
		return false;
	}

	// Insert the output file location
	if (!temporaryPath.InsertExternalFilePath(_oFileLocation))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorInvalidFile);
		return false;
	}

	// m_IteratorPath.GetTemporaryPathStringFull(), m_IteratorPath.GetTemporaryPathFilename(), m_IteratorPath.GetTemporaryPathActionDirectory(), _oFileLocation
	return GetAux(temporaryPath);
}

bool Packet::PacketObjectIterator::MakeDir(std::string _dirPath)
{
	// Check if the dir is a folder path
	if (!PacketStringOperations::PathIsFolder(_dirPath))
	{
		m_ErrorObject.Set(PacketErrorPathNotDirectory);
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
	std::vector<std::string> currentDirectory = m_IteratorPath.ComposeActionDirectory(_dirPath);

	// For each folder
	for (auto & folder : dirPathVec)
	{
		// Check if we already have a file on that location
		if (!m_PacketStructureReference.DirectoryFromPathIsValid(currentDirectory, folder))
		{
			// Try to create the new folder
			if (!m_PacketStructureReference.InsertFolder(folder, currentDirectory))
			{
				// Set the error
				m_ErrorObject.Set(PacketErrorFolderCreationFailed);
				return false;
			}
		}

		// Move to the created directory
		currentDirectory.push_back(folder);
	}

	return true;
}

bool Packet::PacketObjectIterator::Delete(std::string _iLocation)
{
	// Check if the _iLocation is a file
	if (PacketStringOperations::PathIsFile(_iLocation))
	{
		return DeleteFile(_iLocation);
	}

	// Check if the _iLocation is a folder
	if (PacketStringOperations::PathIsFolder(_iLocation))
	{
		return DeleteFolder(_iLocation);
	}

	// Invalid input path
	return false;

	// TODO: CHECK IF THE PATH IS A FILE OR FOLDER
	return false;

	// Get the file name from the path
	std::string fileName = PacketStringOperations::GetFilenameFromPath(_iLocation);

	// Get the file directory only
	std::string fileDirectory = PacketStringOperations::GetDirectoryFromPath(_iLocation);

	// Compose the action directory
	std::vector<std::string> actionDirectory = m_IteratorPath.ComposeActionDirectory(fileDirectory, true);

	// Compose the string directory
	std::string stringDir = PacketStringOperations::ComposeDirectory(actionDirectory);

	// Check if we have a file on that location
	if (!m_PacketStructureReference.FileFromPathIsValid(actionDirectory, fileName))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorFileFromPathDuplicated);
		return false;
	}

	// Get the file fragment identifier and check if it is valid
	PacketObjectManager::FileFragmentIdentifier* fileFragmentIdentifier = m_PacketHashTableReference.GetEntry(stringDir + fileName);
	if (fileFragmentIdentifier == nullptr)
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorInvalidFile);
		return false;
	}

	// Remove the file
	if (!m_PacketManagerReference.RemoveFile(*fileFragmentIdentifier))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorDeleteFile);
		return false;
	}

	// Remove the hash reference
	if (!m_PacketHashTableReference.RemoveEntry(stringDir + fileName))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorDeleteHashEntry);
		return false;
	}

	// Remove the structure reference
	// if(!m_PacketStructureReference.Remove)
	return true;
}

bool Packet::PacketObjectIterator::DeleteFile(std::string _iFileLocation)
{
	return true;
}

bool Packet::PacketObjectIterator::DeleteFolder(std::string _iFolderLocation)
{
	return true;
}

std::vector<std::string> Packet::PacketObjectIterator::List()
{
	std::vector<std::string> folderList = m_PacketStructureReference.GetFolderList(m_IteratorPath.GetCurrentActionPath());
	std::vector<std::string> fileList = m_PacketStructureReference.GetFileList(m_IteratorPath.GetCurrentActionPath());
	folderList.insert(std::end(folderList), std::begin(fileList), std::end(fileList));
	return folderList;
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

	std::vector<std::string> folderList = m_PacketStructureReference.GetFolderList(splitPath);
	std::vector<std::string> fileList = m_PacketStructureReference.GetFileList(splitPath);
	folderList.insert(std::end(folderList), std::begin(fileList), std::end(fileList));
	return folderList;
}

std::string Packet::PacketObjectIterator::GetCurrentPath()
{
	std::string result = m_PacketStructureReference.GetRootName() + ":\\";
	std::vector<std::string> currentActionDirectory = m_IteratorPath.GetCurrentActionPath();

	// For each folder
	for (unsigned int i=0; i<currentActionDirectory.size(); i++)
	{
		// Add it to the result
		result += currentActionDirectory[i];
		if (i != currentActionDirectory.size() - 1) result += '\\';
	}
	
	return result;
}

Packet::PacketError Packet::PacketObjectIterator::GetError()
{
	return m_ErrorObject;
}

// std::string _outputFilePath, std::string _fileName, std::vector<std::string> _dir, std::string iFolderLocation
bool Packet::PacketObjectIterator::PutAux(PacketObjectTemporaryPath& _temporaryPath)
{
	// Check if we can add this
	if (m_PacketHashTableReference.EntryExist(_temporaryPath.GetFullPath()))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorHashDuplicate);
		return false;
	}
	
	// Check if we already have a file on that location
	if (m_PacketStructureReference.FileFromPathIsValid(_temporaryPath.GetFolderSplitPath(), _temporaryPath.GetFilename()))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorFileFromPathInvalid);
		return false;
	}

	// Insert the new file inside the object data
	PacketObjectManager::FileFragmentIdentifier fileFragmentIdentifier;
	if (!m_PacketManagerReference.InsertFile(_temporaryPath.GetExternalFilePath(), fileFragmentIdentifier))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorInvalidFileData);
		return false;
	}

	// Insert into the hash table
	PacketObjectHashTable::PacketObjectHash fileHashIdentifier;
	if (!m_PacketHashTableReference.InsertEntry(_temporaryPath.GetFullPath(), fileFragmentIdentifier, fileHashIdentifier))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorHashDuplicate);
		return false;
	}

	// Insert the new file inside the current folder
	if (!m_PacketStructureReference.InsertFile(_temporaryPath.GetFilename(), fileHashIdentifier, _temporaryPath.GetFolderSplitPath()))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorStructureInsert);
		return false;
	}

	return true;
}

#include <iostream>

// std::string _internalFilePath, std::string _fileName, std::vector<std::string> _dir, std::string _outputFilePath
bool Packet::PacketObjectIterator::GetAux(PacketObjectTemporaryPath& _temporaryPath)
{
	// Check if we have a file on that location
	if (!m_PacketStructureReference.FileFromPathIsValid(_temporaryPath.GetFolderSplitPath(), _temporaryPath.GetFilename()))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorFileFromPathDuplicated);
		return false;
	}

	// Get the file fragment identifier and check if it is valid
	PacketObjectManager::FileFragmentIdentifier* fileFragmentIdentifier = m_PacketHashTableReference.GetEntry(_temporaryPath.GetFullPath());
	if (fileFragmentIdentifier == nullptr)
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorInvalidFile);
		return false;
	}

	// Get the output directory from the output path
	std::string outputDirectory = _temporaryPath.GetExternalFilePath(); // _outputFilePath + "\\" + _fileName;

	// Get the file
	if (!m_PacketManagerReference.GetFile(outputDirectory, *fileFragmentIdentifier))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorRetrieveData);
		return false;
	}

	return true;
}