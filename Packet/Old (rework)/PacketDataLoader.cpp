////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketDataLoader.h"
#include "PacketObject.h"
#include "PacketObjectManager.h"

#include <chrono>
#include <cassert>

Packet::PacketDataLoader::PacketDataLoader(PacketObject* _packetObject)
{
	// Set our initial data
	m_PacketObjectReference = _packetObject;
}

Packet::PacketDataLoader::~PacketDataLoader()
{
}

uint32_t Packet::PacketDataLoader::GetFileSize(Packet::PacketFragment::FileIdentifier _fileIdentifier)
{
	// Lock the mutex
	std::lock_guard<std::mutex> guard(m_LoadingMutex);

	// Get the file metadata
	uint32_t fileSize;
	PacketObjectManager::FileFragmentIdentifier fileFragmentIdentifier;
	if (!GetFileMetadata(_fileIdentifier, fileSize, fileFragmentIdentifier))
	{
		return false;
	}

	// Return the size
	return fileSize;
}

bool Packet::PacketDataLoader::GetFileData(Packet::PacketFragment::FileIdentifier _fileIdentifier, unsigned char* _dataLocation)
{
	// Lock the mutex
	std::lock_guard<std::mutex> guard(m_LoadingMutex);
	
	// Get the file manager reference from our packet object
	PacketObjectManager* objectManagerReference = m_PacketObjectReference->GetObjectManagerReference();

	// Get the file metadata
	uint32_t fileSize;
	PacketObjectManager::FileFragmentIdentifier fileFragmentIdentifier;
	if (!GetFileMetadata(_fileIdentifier, fileSize, fileFragmentIdentifier))
	{
		return false;
	}

	//
	assert(fileSize != 0);

	// Load the file
	uint32_t dataSize = 0;
	bool result = objectManagerReference->GetData(_dataLocation, dataSize, fileFragmentIdentifier);
	if (!result)
	{
		return false;
	}

	return true;
}

bool Packet::PacketDataLoader::GetFileMetadata(PacketFragment::FileIdentifier _fileIdentifier, uint32_t& _fileSize, PacketObjectManager::FileFragmentIdentifier& _fileFragmentIdentifier)
{
	// Get the file manager and hash table references from our packet object
	PacketObjectManager* objectManagerReference = m_PacketObjectReference->GetObjectManagerReference();
	PacketObjectHashTable* objectHashTableReference = m_PacketObjectReference->GetObjectHashTableReference();

	// Get the file fragment identifier
	Packet::PacketObjectManager::FileFragmentIdentifier* fileFragmentIdentifier = objectHashTableReference->GetEntry(_fileIdentifier);
	if (fileFragmentIdentifier == nullptr)
	{
		return false;
	}

	// Set the fragment identifier variable
	_fileFragmentIdentifier = *fileFragmentIdentifier;

	// Get the file size
	uint32_t fileSize = objectManagerReference->GetFileSize(_fileFragmentIdentifier);
	if (fileSize == 0)
	{
		return false;
	}

	// Set the file size variable
	_fileSize = fileSize;

	return true;
}