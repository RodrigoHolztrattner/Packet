////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileRemover.h"

Packet::PacketFileRemover::PacketFileRemover(PacketFileStorage& _packetFileStorage) : 
	m_FileStorageReference(_packetFileStorage)
{
	// Create the thread for the threaded load routine
	m_DeletionThread = std::thread(&Packet::PacketFileRemover::ThreadedDeletionRoutine, this);
}

Packet::PacketFileRemover::~PacketFileRemover()
{
}

void Packet::PacketFileRemover::TryRemoveFile(PacketFileReference* _fileReference)
{
	// Get the file object from the reference
	PacketFile* fileObject = _fileReference->GetFileObject();
	
	// Insert the file identifier we will try to remove
	m_ReleaseRequestQueue.Insert(fileObject->GetFileIdentifier());
}

bool Packet::PacketFileRemover::UseThreadedQueue(uint32_t _totalNumberMaximumThreads, std::function<uint32_t()> _threadIndexMethod)
{
	// Set to use the threaded queue mode
	return m_ReleaseRequestQueue.AllowThreadedAccess(_totalNumberMaximumThreads, _threadIndexMethod);
}

void Packet::PacketFileRemover::ProcessFileQueues()
{
	// Prevent multiple threads from running this code (only one thread allowed, take care!)
	std::lock_guard<std::mutex> guard(m_Mutex);

	// For each deletion request
	m_ReleaseRequestQueue.ProcessAll([&](Packet::PacketFragment::FileIdentifier& _fileIdentifier)
	{
		// Call the process method
		ProcessRemovalRequest(_fileIdentifier);
	}, true);
}

bool Packet::PacketFileRemover::ProcessRemovalRequest(PacketFragment::FileIdentifier _fileIdentifier)
{
	// Everything is thread safe from here, this method is called only by the ProcessFileQueues() method and it has a mutex to
	// prevent multiple threads simultaneously
	
	// Release this file
	PacketFile* file = m_FileStorageReference.TryReleaseFileFromIdentifier(_fileIdentifier);
	if (file != nullptr)
	{
		// Insert the file into the deletion queue
		m_DeletionQueue.Enqueue(file);
	}

	return true;
}

void Packet::PacketFileRemover::ThreadedDeletionRoutine()
{
	// While forever
	while (true)
	{
		// Try do dequeue an item
		PacketFile* file = m_DeletionQueue.TryDequeue();
		if (file == nullptr)
		{
			// Yield
			// std::this_thread::yield();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		else
		{
			// Call the release method
			file->Release();

			// Delete this file
			delete file; // TODO use another deallocation method
		}
	}
}