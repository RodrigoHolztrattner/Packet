////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileRequester.h"
#include "PacketObject.h"
#include "PacketFileLoader.h"

Packet::PacketFileRequester::PacketFileRequester(PacketObject* _packetObject) : m_PacketFileLoader(_packetObject)
{
	// Set our initial data
	m_UseThreadQueue = false;
	m_MaximumTotalThreadedQueues = 0;
}

Packet::PacketFileRequester::~PacketFileRequester()
{
}

bool Packet::PacketFileRequester::UseThreadedQueue(uint32_t _totalNumberMaximumThreads, std::function<uint32_t()> _threadIndexMethod)
{
	// Check if the number of thread is at last 1
	if (_totalNumberMaximumThreads <= 0)
	{
		return false;
	}

	// Set the threaded data
	m_MaximumTotalThreadedQueues = _totalNumberMaximumThreads;
	m_ThreadIndexMethod = _threadIndexMethod;

	// Set that we use thread queues
	m_UseThreadQueue = true;

	return true;
}

Packet::PacketFile* Packet::PacketFileRequester::RequestFile(PacketFragment::FileIdentifier _fileIdentifier, PacketFile::DispatchType _dispatchType, bool _delayAllocation)
{
	// Create a new file object // TODO essa alocacao talvez deva ser diferente ou permitir custom allocator
	PacketFile* newFileObject = new PacketFile(_fileIdentifier, _dispatchType, _delayAllocation);

	// Prepare the request data
	FileRequestData requestData = {};
	requestData.fileIdentifier = _fileIdentifier;
	requestData.fileReference = newFileObject;

	// Check if the dispatch type is on request
	if (_dispatchType == PacketFile::DispatchType::OnRequest)
	{
		// Load this file right now
		if (!ProcessRequest(requestData))
		{
			// Error TODO call
			return nullptr;
		}

		return newFileObject;
	}

	// Check if we should use a thread queue and if the thread index method was set
	if (m_UseThreadQueue && m_ThreadIndexMethod)
	{
		// Get the thread index we should use
		uint32_t threadIndex = m_ThreadIndexMethod();

		// Check if the thread index is valid
		if (threadIndex >= 0 && threadIndex < m_MaximumTotalThreadedQueues)
		{
			// Insert this file into the selected queue (no need to use mutex because each thread will access only
			// their respective queue
			m_ThreadedRequestQueues[threadIndex].push_back(requestData);
		}
	}
	// Use the base queue
	else
	{
		// Lock our mutex (only one thread allowed from there)
		std::lock_guard<std::mutex> guard(m_Mutex);

		// Insert this file into the base queue
		m_BaseRequestQueue.push_back(requestData);
	}

	return newFileObject;
}

bool Packet::PacketFileRequester::ProcessLoadingQueues()
{
	// For now we will assume that no thread will try to request new files, NOT thread safe from threre! //
	
	// For each request on the base queue
	for (auto& request : m_BaseRequestQueue)
	{
		// Load this request
		if (!ProcessRequest(request))
		{
			// Error TODO throw
			return false;
		}
	}

	// Clear the base request queue
	m_BaseRequestQueue.clear();

	// If we should use the threaded queues
	if (m_UseThreadQueue)
	{
		// For each threaded queue
		for (int i=0; i<m_MaximumTotalThreadedQueues; i++)
		{
			// Get a reference to this queue
			auto& threadedQueue = m_ThreadedRequestQueues[i];

			// For each request inside this queue
			for (auto& request : threadedQueue)
			{
				// Load this request
				if (!ProcessRequest(request))
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

bool Packet::PacketFileRequester::ProcessRequest(FileRequestData _requestData)
{
	// Check if we already have this file on our hash (mudar esse nome)
	// TODO
	
	// Pass this file to the file loader
	if (!m_PacketFileLoader.ProcessPacketFile(_requestData.fileReference))
	{
		return false;
	}

	return true;
}