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
	// Check the allocation type
	if (!_packetFile->AllocationIsDelayed())
	{
		// Get the file metadata
		uint32_t fileSize;
		PacketObjectManager::FileFragmentIdentifier fileFragmentIdentifier;
		if (!GetFileData(_packetFile->GetFileIdentifier(), fileSize, fileFragmentIdentifier))
		{
			return false;
		}

		// Allocate the file data
		_packetFile->AllocateData(fileSize);
	}

	// Check if the file dispatch is not assync (if it is OnRequest or OnProcess)
	if (_packetFile->GetDispatchType() != PacketFile::DispatchType::Assync)
	{
		// Load this file now
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
	uint32_t fileSize;
	PacketObjectManager::FileFragmentIdentifier fileFragmentIdentifier;
	if (!GetFileData(_file->GetFileIdentifier(), fileSize, fileFragmentIdentifier))
	{
		return false;
	}

	// Check the allocation type
	if (_file->AllocationIsDelayed())
	{
		// Allocate the file data
		_file->AllocateData(fileSize);
	}

	// Lock the mutex
	std::lock_guard<std::mutex> guard(m_LoadingMutex);

	// Load the file
	uint32_t dataSize = 0;
	bool result = objectManagerReference->GetData(_file->GetDataPtr(), dataSize, fileFragmentIdentifier);
	if(!result)
	{
		return false;
	}

	// Set the data size
	_file->SetDataSize(dataSize);

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
				std::logic_error("PacketFileLoader -> Failed to load the current file");
			}
		}
	}
}