////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileRemover.h"

Packet::PacketFileRemover::PacketFileRemover(PacketFileStorage& _packetFileStorage) : m_PacketFileStorageReference(_packetFileStorage)
{
	// Set our initial data
	// ...
}

Packet::PacketFileRemover::~PacketFileRemover()
{
}

bool Packet::PacketFileRemover::TryRemoveFile(Packet::PacketFragment::FileIdentifier _fileIdentifier)
{
	// Check if we are using multiple thread queues
	if (m_UseThreadQueue && m_ThreadIndexMethod)
	{
		// Get the thread index we should use
		uint32_t threadIndex = m_ThreadIndexMethod();

		// Check if the thread index is valid
		if (threadIndex >= 0 && threadIndex < m_MaximumTotalThreadedQueues)
		{
			// Insert this file into the selected queue (no need to use mutex because each thread will access only
			// their respective queue
			m_ThreadedDeletionQueues[threadIndex].push_back(_fileIdentifier);
		}
	}
	// Use the base queue
	else
	{
		// Lock our mutex (only one thread allowed from there)
		std::lock_guard<std::mutex> guard(m_Mutex);

		// Insert this file into the base queue
		m_BaseDeletionQueue.push_back(_fileIdentifier);
	}

	return true;
}


bool Packet::PacketFileRemover::UseThreadedQueue(uint32_t _totalNumberMaximumThreads, std::function<uint32_t()> _threadIndexMethod)
{
	// Check if the number of thread is at last 1
	if (_totalNumberMaximumThreads <= 0)
	{
		return false;
	}

	// Set the threaded data
	m_MaximumTotalThreadedQueues = _totalNumberMaximumThreads;
	m_ThreadIndexMethod = _threadIndexMethod;

	// Allocate enough queues for all threads
	m_ThreadedDeletionQueues = new std::vector<PacketFragment::FileIdentifier>[m_MaximumTotalThreadedQueues];

	// Set that we use thread queues
	m_UseThreadQueue = true;

	return true;
}

bool Packet::PacketFileRemover::ProcessFileQueues()
{
	// For now we will assume that no thread will try to request new files, NOT thread safe from threre! //

	// For each request on the base queue
	for (auto& request : m_BaseDeletionQueue)
	{
		// Process this request
		if (!ProcessRemovalRequest(request))
		{
			// Error TODO throw
			return false;
		}
	}

	// Clear the base request queue
	m_BaseDeletionQueue.clear();

	// If we should use the threaded queues
	if (m_UseThreadQueue)
	{
		// For each threaded queue
		for (int i = 0; i<m_MaximumTotalThreadedQueues; i++)
		{
			// Get a reference to this queue
			auto& threadedQueue = m_ThreadedDeletionQueues[i];

			// For each request inside this queue
			for (auto& request : threadedQueue)
			{
				// Process this request
				if (!ProcessRemovalRequest(request))
				{
					// Error TODO throw
					return false;
				}
			}

			// Clear this request queue
			threadedQueue.clear();
		}
	}

	return true;
}

bool Packet::PacketFileRemover::ProcessRemovalRequest(PacketFragment::FileIdentifier _fileIdentifier)
{
	// Everything is single threaded from here //
	
	// Shutdown this file
	if (!m_PacketFileStorageReference.ShutdownFileFromIdentifier(_fileIdentifier))
	{
		return false;
	}

	return true;
}