////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketLoader.h"

Packet::PacketLoader::PacketLoader()
{
	// Set the initial data
	m_OperationMode = PacketMode::Vault;
}

Packet::PacketLoader::~PacketLoader()
{
}

bool Packet::PacketLoader::LoadFile(PacketFile* _fileInfo, std::vector<unsigned char>& _byteArray)
{
	// Check for editor mode
	if (m_OperationMode == PacketMode::Editor)
	{
		// Load from raw
		return LoadRawFile(_fileInfo, _byteArray);
	}

	/*
	// Open the index file
	std::ifstream indexFile(m_IndexFileName, std::ios::in | std::ios::binary);
	if (!indexFile.is_open())
	{
		// We didnt found a valid index file... using the dummy one...
		return;
	}
	*/

	return false; // Not implemented, yet

	return true;
}

bool Packet::PacketLoader::SaveFileIntoVault(PacketFile* _fileInfo, std::vector<unsigned char>& _byteArray)
{
	// We can only save a file in editor mode
	if (m_OperationMode != PacketMode::Editor)
	{
		// Wrong mdoe!
		return false;
	}
	/*
	// Create the new index file
	std::ofstream indexFile(_fileInfo.filePath.GetString(), std::ios::out | std::ios::binary);
	if (!indexFile.is_open())
	{
		// Problem creating the file!
		return;
	}
	*/

	return false; // Not implemented, yet
	return true;
}

#define GetSizeFromFile(filename, size)										\
{																			\
	std::ifstream file(filename, std::ios::binary | std::ios::ate);			\
	size = file.tellg();													\
}

bool Packet::PacketLoader::LoadRawFile(PacketFile* _fileInfo, std::vector<unsigned char>& _byteArray)
{
	// Get the file size
	uint32_t fileSize;
	GetSizeFromFile(_fileInfo->fileExternalPath.GetString(), fileSize);

	// Alloc space for the data
	_byteArray.resize(fileSize);

	// Open the index file
	std::ifstream rawFile(_fileInfo->fileExternalPath.GetString(), std::ios::in | std::ios::binary);
	if (!rawFile.is_open())
	{
		// We didnt found a valid file
		return false;
	}

	// Read the file data
	rawFile.read((char*)_byteArray.data(), fileSize);

	// Close the file
	rawFile.close();

	return true;
}