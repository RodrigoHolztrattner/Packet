////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileRequester.h"
#include "PacketObject.h"
#include "PacketFileLoader.h"

Packet::PacketFileRequester::PacketFileRequester(PacketObject* _packetObject) : 
	m_PacketFileLoader(_packetObject), 
	m_PacketFileRemover(m_PacketFileStorage)
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

	// Allocate enough queues for all threads
	m_ThreadedRequestQueues = new std::vector<FileRequestData>();

	// Set that we use thread queues
	m_UseThreadQueue = true;

	// Set the file remover to use threads too
	if (!m_PacketFileRemover.UseThreadedQueue(_totalNumberMaximumThreads, _threadIndexMethod))
	{
		return false;
	}

	return true;
}

bool Packet::PacketFileRequester::RequestFile(FutureReference<PacketFile>* _futureObject, PacketFragment::FileIdentifier _fileIdentifier, PacketFile::DispatchType _dispatchType, bool _delayAllocation)
{
	// Prepare the request data
	FileRequestData requestData = {};
	requestData.fileReference = _futureObject;
	requestData.fileIdentifier = _fileIdentifier;
	requestData.fileDispatchType = _dispatchType;
	requestData.delayAllocation = _delayAllocation;

	// Check if the dispatch type is on request and we should process the request right now
	if (_dispatchType == PacketFile::DispatchType::OnRequest)
	{
		// Load this file right now
		if (!ProcessRequest(requestData))
		{
			// Error TODO call
			return false;
		}

		return true;
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

	return true;
}

bool Packet::PacketFileRequester::RequestFile(FutureReference<PacketFile>* _futureObject, const char* _fileName, PacketFile::DispatchType _dispatchType, bool _delayAllocation)
{
	return RequestFile(_futureObject, HashFilePathStatic(_fileName), _dispatchType, _delayAllocation);
}

bool Packet::PacketFileRequester::ProcessFileQueues()
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

	// Process the file queues for the remover too
	if (!m_PacketFileRemover.ProcessFileQueues())
	{
		return false;
	}

	return true;
}

bool Packet::PacketFileRequester::ProcessRequest(FileRequestData _requestData)
{
	// Everything is single threaded from here //

	// Check if we already have this file on our storage
	PacketFile* file = m_PacketFileStorage.RequestFileFromIdentifier(_requestData.fileIdentifier);
	if (file != nullptr)
	{
		// Ok the file was already loaded, the reference count incremented and we are ready to go
		_requestData.fileReference->SetInternalObject(file);

		return true;
	}

	// Create a new file object
	PacketFile* newFile = new PacketFile(&m_PacketFileRemover, _requestData.fileIdentifier, _requestData.fileDispatchType, _requestData.delayAllocation);
	// TODO use custom allocator for the new file?
	
	// Insert this new file into the storage
	if (!m_PacketFileStorage.InserFileWithIdentifier(_requestData.fileIdentifier, newFile))
	{
		return false;
	}

	// Set the file reference internal object
	_requestData.fileReference->SetInternalObject(newFile);

	// Pass this file to the file loader
	if (!m_PacketFileLoader.ProcessPacketFile(newFile))
	{
		return false;
	}

	return true;
}