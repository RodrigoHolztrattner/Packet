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
}

Packet::PacketFileRequester::~PacketFileRequester()
{
}

bool Packet::PacketFileRequester::UseThreadedQueue(uint32_t _totalNumberMaximumThreads, std::function<uint32_t()> _threadIndexMethod)
{
	// Allow thread access for our queue
	if (!m_RequestQueue.AllowThreadedAccess(_totalNumberMaximumThreads, _threadIndexMethod))
	{
		return false;
	}

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

	// Insert our request data
	m_RequestQueue.Insert(requestData);

	return true;
}

bool Packet::PacketFileRequester::RequestFile(FutureReference<PacketFile>* _futureObject, const char* _fileName, PacketFile::DispatchType _dispatchType, bool _delayAllocation)
{
	return RequestFile(_futureObject, HashFilePathStatic(_fileName), _dispatchType, _delayAllocation);
}

void Packet::PacketFileRequester::ProcessFileQueues()
{
	// For each request, run the process method
	m_RequestQueue.ProcessAll([&](FileRequestData& _requestData)
	{
		// Process this request
		ProcessRequest(_requestData);
	}, true);
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