////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketError.h"
#include "PacketFileDataOperations.h"
#include "PacketStringOperations.h"

#include <iostream>
#include <experimental/filesystem>

Packet::PacketError::PacketError()
{
	// Set the initial data
	m_ErrorIdentifier = PacketErrorNoError;
	m_IsSet = false;
}

Packet::PacketError::~PacketError()
{
}

void Packet::PacketError::Set(uint32_t _errorIdentifier)
{
	// Set the error identifier
	m_ErrorIdentifier = _errorIdentifier;

	// Set the message
	switch (_errorIdentifier)
	{
		// No error
		case PacketErrorNoError:				{m_ErrorMessage = "No error found"; break; }

		// Iterator errors
		case PacketErrorPathNotDirectory:		{m_ErrorMessage = "The given path isn't a valid directory"; break; }
		case PacketErrorPathNotFile:			{m_ErrorMessage = "The given path isn't a valid file"; break; }
		case PacketErrorInvalidDirectory:		{m_ErrorMessage = "The given path does not exist in the current directory or isn't valid"; break; }
		case PacketErrorInvalidFile:			{m_ErrorMessage = "The given file does not exist in the current directory or isn't valid"; break; }
		case PacketErrorHashDuplicate:			{m_ErrorMessage = "The given file already exist inside our hash table"; break; }
		case PacketErrorFileFromPathInvalid:	{m_ErrorMessage = "The file doesn't exist in the current location"; break; }
		case PacketErrorFileFromPathDuplicated: {m_ErrorMessage = "The file already exist in the current location"; break; }
		case PacketErrorInvalidFileData:		{m_ErrorMessage = "Problem to add the file data, check if the size is valid or if the file isn't oppened by another program"; break; }
		case PacketErrorStructureInsert:		{m_ErrorMessage = "Problem when adding the file into our internal strcture system"; break; }
		case PacketErrorRetrieveData:			{m_ErrorMessage = "Problem when retrieving the file data"; break; }
		case PacketErrorFolderCreationFailed:	{m_ErrorMessage = "Folder creation wasn't successful"; break; }		

		// PacketFile errors
		case PacketErrorInvalidFileIdentifier:		{m_ErrorMessage = "The given file identifier seems to be invalid, we can't retrieve the metadata with it"; break; }
		case PacketErrorFileDataAlreadyAllocated:	{m_ErrorMessage = "The file memory was already allocated (possible leak?)"; break; }
	}
	
	// Set
	m_IsSet = true;

	// Check if we should print the current error
	#ifdef PacketErrorVerbose
		PrintInfo(false);
	#endif
}

void Packet::PacketError::Set(uint32_t _errorIdentifier, std::string _errorMessage)
{
	// Set the error identifier and message
	m_ErrorIdentifier = _errorIdentifier;
	m_ErrorMessage = _errorMessage;

	// Set
	m_IsSet = true;

	// Check if we should print the current error
	#ifdef PacketErrorVerbose
		PrintInfo(false);
	#endif
}

bool Packet::PacketError::IsSet()
{
	return m_IsSet;
}

void Packet::PacketError::Clear()
{
	// Set the data
	m_ErrorIdentifier = PacketErrorNoError;
	m_ErrorMessage = std::string();
	m_IsSet = false;
}

bool Packet::PacketError::GetInfo(uint32_t& _errorIdentifier, std::string& _errorMessage, bool _clearCurrentError)
{
	// Check if there is an error
	if (!m_IsSet)
	{
		return false;
	}

	// Set the error info
	_errorIdentifier = m_ErrorIdentifier;
	_errorMessage = m_ErrorMessage;

	// Check if we should clear the current error info
	if (_clearCurrentError)
	{
		// Clear the current error
		Clear();
	}

	return true;
}

bool Packet::PacketError::PrintInfo(bool _clearCurrentError)
{
	// Check if there is an error
	if (!m_IsSet)
	{
		return false;
	}

	// Print the error info
	std::cout << "Packet error code: " << m_ErrorIdentifier << std::endl;
	std::cout << "    - Error message: " << m_ErrorMessage << std::endl << std::endl;

	// Check if we should clear the current error info
	if (_clearCurrentError)
	{
		// Clear the current error
		Clear();
	}

	return true;
}