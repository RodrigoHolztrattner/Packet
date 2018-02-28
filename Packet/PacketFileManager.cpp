////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileManager.h"
#include "PacketObject.h"
#include "PacketFileLoader.h"

Packet::PacketFileManager::PacketFileManager(PacketObject* _packetObject) : 
	m_PacketFileLoader(_packetObject),
	m_PacketFileRequester(m_PacketFileLoader, m_PacketFileStorage),
	m_PacketFileRemover(m_PacketFileStorage)
{
}

Packet::PacketFileManager::~PacketFileManager()
{
}

bool Packet::PacketFileManager::UseThreadedQueue(uint32_t _totalNumberMaximumThreads, std::function<uint32_t()> _threadIndexMethod)
{
	// Allow thread access for our queue
	if (!m_PacketFileRequester.UseThreadedQueue(_totalNumberMaximumThreads, _threadIndexMethod))
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

bool Packet::PacketFileManager::RequestReference(PacketFileReference* _fileReference, PacketFragment::FileIdentifier _fileIdentifier, PacketFile::DispatchType _dispatchType, bool _delayAllocation)
{
	return m_PacketFileRequester.RequestFile(_fileReference, _fileIdentifier, _dispatchType, _delayAllocation);
}

bool Packet::PacketFileManager::RequestReference(PacketFileReference* _fileReference, const char* _fileName, PacketFile::DispatchType _dispatchType, bool _delayAllocation)
{
	return RequestReference(_fileReference, HashFilePathStatic(_fileName), _dispatchType, _delayAllocation);
}

bool Packet::PacketFileManager::ReleaseReference(PacketFileReference* _fileReference)
{
	// Check if the file reference is valid
	if (!_fileReference->IsReady())
	{
		return false;
	}

	// Try to remove the file reference (or just subtract the reference count)
	m_PacketFileRemover.TryRemoveFile(_fileReference);

	// Call the release method for this file reference
	_fileReference->Release();

	return true;
}

void Packet::PacketFileManager::ProcessQueues()
{
	// TODO: We should use a mutex here to prevent multiple threads trying to run simultaneously?
	
	// Process the file queues for the requester
	m_PacketFileRequester.ProcessFileQueues();

	// Call the process method for the file remover too
	m_PacketFileRemover.ProcessFileQueues();
}
