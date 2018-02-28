////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileRequester.h"
#include "PacketObject.h"
#include "PacketFileLoader.h"

Packet::PacketFileRequester::PacketFileRequester(PacketFileLoader& _fileLoaderReference, PacketFileStorage& _fileStorageReference) :
	m_FileLoaderReference(_fileLoaderReference),
	m_FileStorageReference(_fileStorageReference)
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

	return true;
}

bool Packet::PacketFileRequester::RequestFile(PacketFileReference* _fileReference, PacketFragment::FileIdentifier _fileIdentifier, PacketFile::DispatchType _dispatchType, bool _delayAllocation)
{
	// Prepare the request data
	FileRequestData requestData = {};
	requestData.fileReference = _fileReference;
	requestData.fileIdentifier = _fileIdentifier;
	requestData.fileDispatchType = _dispatchType;
	requestData.delayAllocation = _delayAllocation;

	// Check if the dispatch type is on request and we should process the request right now
	if (_dispatchType == PacketFile::DispatchType::OnRequest)
	{
		// Only one thread allowed when the dispatch type is "on request", prefer using another dispatch type for non-blocking operation
		std::lock_guard<std::mutex> guard(m_Mutex);

		// Load this file right now
		if (!ProcessRequest(requestData))
		{
			return false;
		}

		return true;
	}

	// Insert our request data
	m_RequestQueue.Insert(requestData);

	return true;
}

void Packet::PacketFileRequester::ProcessFileQueues()
{
	// Prevent multiple threads from running this code (only one thread allowed, take care!)
	std::lock_guard<std::mutex> guard(m_Mutex);

	// For each request, run the process method
	m_RequestQueue.ProcessAll([&](FileRequestData& _requestData)
	{
		// Process this request
		ProcessRequest(_requestData);
	}, true);
}

bool Packet::PacketFileRequester::ProcessRequest(FileRequestData _requestData)
{
	// Everything is thread-safe from here, this method is only callable from the ProcessFileQueues() and RequestFile() methods
	// and both uses mutexes for syncronization

	// Check if we already have this file on our storage
	PacketFile* file = m_FileStorageReference.RequestFileFromIdentifier(_requestData.fileIdentifier);
	if (file != nullptr)
	{
		// Ok the file was loaded, the reference count incremented and we are ready to go
		_requestData.fileReference->SetFileReference(file);

		// Add teh file reference request
		file->AddFileReferenceRequest(_requestData.fileReference);

		return true;
	}

	// Create a new file object
	PacketFile* newFile = new PacketFile(_requestData.fileIdentifier, _requestData.fileDispatchType, _requestData.delayAllocation);
	// TODO use custom allocator for the new file?
	// TODO move this inside the file storage?
	
	// Insert this new file into the storage
	if (!m_FileStorageReference.InserFileWithIdentifier(_requestData.fileIdentifier, newFile))
	{
		return false;
	}

	// Set the file reference internal object
	_requestData.fileReference->SetFileReference(newFile);

	// Pass this file to the file loader
	if (!m_FileLoaderReference.ProcessPacketFile(newFile))
	{
		return false;
	}

	// Add teh file reference request
	newFile->AddFileReferenceRequest(_requestData.fileReference);

	return true;
}