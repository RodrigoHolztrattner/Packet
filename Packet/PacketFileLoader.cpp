////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileLoader.h"
#include "PacketFile.h"
#include "PacketObject.h"
#include "PacketObjectManager.h"

Packet::PacketFileLoader::PacketFileLoader(PacketObject* _packetObject)
{
	// Set our initial data
	m_PacketObjectReference = _packetObject;

	// Create the thread for the threaded load routine
	m_LoadingThread = loadThread(&Packet::PacketFileLoader::ThreadedLoadRoutine, this);
}

Packet::PacketFileLoader::~PacketFileLoader()
{
}

bool Packet::PacketFileLoader::ProcessPacketFile(PacketFile* _packetFile)
{
	// Get the file manager reference from our packet object
	PacketObjectManager* objectManagerReference = m_PacketObjectReference->GetObjectManagerReference();

	// Check the file dispatch type
	if (_packetFile->GetDispatchType() == PacketFile::DispatchType::Sync)
	{
		// Allocate the file data
		_packetFile->AllocateData();

		// Load this file
		return LoadFile(_packetFile);
	}

	// Insert it into our load queue
	m_LoadQueue.Enqueue(_packetFile);

	return true;
}

unsigned int Packet::PacketFileLoader::GetFileSize(PacketObjectManager::FileFragmentIdentifier* _fragmentFileIdentifier)
{
	// Get the file manager reference from our packet object
	PacketObjectManager* objectManagerReference = m_PacketObjectReference->GetObjectManagerReference();

	return objectManagerReference->GetFileSize(*_fragmentFileIdentifier);
}

Packet::PacketObjectManager::FileFragmentIdentifier* Packet::PacketFileLoader::GetFileFragmentIdentifier(PacketFragment::FileIdentifier _fileIdentifier)
{
	// Get the file manager reference from our packet object
	PacketObjectManager* objectManagerReference = m_PacketObjectReference->GetObjectManagerReference();

	return m_PacketObjectReference->GetObjectHashTableReference()->GetEntry(_fileIdentifier);;
}

bool Packet::PacketFileLoader::LoadFile(PacketFile* _file)
{
	// Get the file manager reference from our packet object
	PacketObjectManager* objectManagerReference = m_PacketObjectReference->GetObjectManagerReference();

	// Get the file metadata
	PacketFile::Metadata fileMetadata = _file->GetMetadata();

	// Lock the mutex
	std::lock_guard<std::mutex> guard(m_LoadingMutex);

	// Load the file
	bool result = objectManagerReference->GetData(_file->GetInternalDataPtr(), fileMetadata->fileFragmentIdentifier);
	if(!result)
	{
		return false;
	}

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
			std::this_thread::yield();
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