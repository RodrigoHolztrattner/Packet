////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileRemover.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketError.h"
#include "PacketMultipleQueue.h"
#include "PacketFile.h"
#include "PacketFileStorage.h"

#include <map>
#include <mutex>

///////////////
// NAMESPACE //
///////////////

/////////////
// DEFINES //
/////////////

////////////
// GLOBAL //
////////////

///////////////
// NAMESPACE //
///////////////

// Packet data explorer
PacketNamespaceBegin(Packet)

////////////////
// FORWARDING //
////////////////

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFileRemover
////////////////////////////////////////////////////////////////////////////////
class PacketFileRemover
{

private:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFileRemover(PacketFileStorage& _packetFileStorage);
	~PacketFileRemover();

//////////////////
// MAIN METHODS //
public: //////////

	// Try to remove a file (will just reduce the reference count if there are more references)
	void TryRemoveFile(PacketFragment::FileIdentifier _fileIdentifier);

	// Set to use multiple thread queues
	bool UseThreadedQueue(uint32_t _totalNumberMaximumThreads, std::function<uint32_t()> _threadIndexMethod);

	// Process all file queues
	void ProcessFileQueues();

private:

	// Process a removal request
	bool ProcessRemovalRequest(PacketFragment::FileIdentifier _fileIdentifier);

///////////////
// VARIABLES //
private: //////

	// The packet file storage reference
	PacketFileStorage& m_PacketFileStorageReference;

	// Our deletion queue
	Packet::MultipleQueue<PacketFragment::FileIdentifier> m_RequestQueue;
};

// Packet data explorer
PacketNamespaceEnd(Packet)