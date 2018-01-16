////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileLoader.h"
#include "PacketFile.h"
#include "PacketObject.h"
#include "PacketObjectManager.h"

#include <chrono>

Packet::PacketFileLoader::PacketFileLoader(PacketObject* _packetObject)
{
	// Set our initial data
	m_PacketObjectReference = _packetObject;

	// Create the thread for the threaded load routine
	m_LoadingThread = std::thread(&Packet::PacketFileLoader::ThreadedLoadRoutine, this);
}

Packet::PacketFileLoader::~PacketFileLoader()
{
}

bool Packet::PacketFileLoader::ProcessPacketFile(PacketFile* _packetFile)
{
	// Get the file manager reference from our packet object
	PacketObjectManager* objectManagerReference = m_PacketObjectReference->GetObjectManagerReference();

	// Check the allocation type
	if (!_packetFile->AllocationIsDelayed())
	{
		// Allocate the file data
		_packetFile->AllocateData();
	}

	// Check the file dispatch type
	if (_packetFile->GetDispatchType() == PacketFile::DispatchType::Sync)
	{
		// Load this file
		return LoadFile(_packetFile);
	}

	// Insert it into our load queue
	m_LoadQueue.Enqueue(_packetFile);

	return true;
}

bool Packet::PacketFileLoader::GetFileData(PacketFragment::FileIdentifier _fileIdentifier, uint32_t& _fileSize, PacketObjectManager::FileFragmentIdentifier& _fileFragmentIdentifier)
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

bool Packet::PacketFileLoader::LoadFile(PacketFile* _file)
{
	// Get the file manager reference from our packet object
	PacketObjectManager* objectManagerReference = m_PacketObjectReference->GetObjectManagerReference();

	// Get the file metadata
	PacketFile::Metadata fileMetadata = _file->GetMetadata();

	// Check the allocation type
	if (_file->AllocationIsDelayed())
	{
		// Allocate the file data
		_file->AllocateData();
	}

	// Lock the mutex
	std::lock_guard<std::mutex> guard(m_LoadingMutex);

	// Load the file
	bool result = objectManagerReference->GetData(_file->GetInternalDataPtr(), fileMetadata.fileFragmentIdentifier);
	if(!result)
	{
		return false;
	}

	// Call the finish loading method for this file
	_file->FinishLoading();

	return true;
}

void Packet::PacketFileLoader::ThreadedLoadRoutine()
{
	// While forever
	while(true)
	{
		// Try do dequeue an item
		PacketFile* file = m_LoadQueue.TryDequeue();
		if(file == nullptr)
		{
			// Yield
			// std::this_thread::yield();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		else
		{
			// Load the file
			bool result = LoadFile(file);
			if(!result)
			{
				// Error (throw)
				// ...
			}
		}
	}
}