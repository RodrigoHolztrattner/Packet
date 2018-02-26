////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileRemover.h"

Packet::PacketFileRemover::PacketFileRemover(PacketFileStorage& _packetFileStorage) : 
	m_FileStorageReference(_packetFileStorage)
{
	// Set our initial data
	// ...
}

Packet::PacketFileRemover::~PacketFileRemover()
{
}

void Packet::PacketFileRemover::TryRemoveFile(PacketFileReference* _fileReference)
{
	// Get the file object from the reference
	PacketFile* fileObject = _fileReference->GetFileObject();
	
	// Insert the file identifier we will try to remove
	m_RequestQueue.Insert(fileObject->GetFileIdentifier());
}

bool Packet::PacketFileRemover::UseThreadedQueue(uint32_t _totalNumberMaximumThreads, std::function<uint32_t()> _threadIndexMethod)
{
	// Set to use the threaded queue mode
	return m_RequestQueue.AllowThreadedAccess(_totalNumberMaximumThreads, _threadIndexMethod);
}

void Packet::PacketFileRemover::ProcessFileQueues()
{
	// For each deletion request
	m_RequestQueue.ProcessAll([&](Packet::PacketFragment::FileIdentifier& _fileIdentifier) 
	{
		// Call the process method
		ProcessRemovalRequest(_fileIdentifier);
	}, true);
}

bool Packet::PacketFileRemover::ProcessRemovalRequest(PacketFragment::FileIdentifier _fileIdentifier)
{
	// Everything is single threaded from here //
	
	// Shutdown this file
	if (!m_FileStorageReference.ShutdownFileFromIdentifier(_fileIdentifier))
	{
		return false;
	}

	return true;
}