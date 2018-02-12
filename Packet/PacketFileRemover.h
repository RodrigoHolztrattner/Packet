////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileRemover.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketError.h"
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
	bool TryRemoveFile(PacketFragment::FileIdentifier _fileIdentifier);

	// Set to use multiple thread queues
	bool UseThreadedQueue(uint32_t _totalNumberMaximumThreads, std::function<uint32_t()> _threadIndexMethod);

	// Process all file queues
	bool ProcessFileQueues();

private:

	// Process a removal request
	bool ProcessRemovalRequest(PacketFragment::FileIdentifier _fileIdentifier);

///////////////
// VARIABLES //
private: //////

	// If we should use multiple threaded queues <check the varaible with the same name on the PacketFileRequester class>
	bool m_UseThreadQueue;

	// The maximum of total threaded queues
	uint32_t m_MaximumTotalThreadedQueues;

	// The threaded index method <used to retrieve the current running thread index>
	std::function<uint32_t()> m_ThreadIndexMethod;

	// The mutex we will use to secure thread safe
	std::mutex m_Mutex;

	// The packet file storage reference
	PacketFileStorage& m_PacketFileStorageReference;

	// Our base deletion queue
	std::vector<PacketFragment::FileIdentifier> m_BaseDeletionQueue; // TODO usar algum vetor com memory allocation diferente? Setar size inicial? Fazer com que ele nunca recue no size?

	// The threaded deletion queues
	std::vector<PacketFragment::FileIdentifier>* m_ThreadedDeletionQueues; // TODO usar algum vetor com memory allocation diferente? Setar size inicial? Fazer com que ele nunca recue no size?
};

// Packet data explorer
PacketNamespaceEnd(Packet)