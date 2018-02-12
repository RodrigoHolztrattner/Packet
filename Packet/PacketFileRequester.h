////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileRequester.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketError.h"
#include "PacketFileLoader.h"
#include "PacketFile.h"

#include <string>
#include <functional>

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
// Class name: PacketFileRequester
////////////////////////////////////////////////////////////////////////////////
class PacketFileRequester
{
public:

private:

	// Our file request type
	struct FileRequestData
	{
		// The file reference
		PacketFile* fileReference;

		// The file identifier
		PacketFragment::FileIdentifier fileIdentifier;
	};

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFileRequester(PacketObject* _packetObject);
	~PacketFileRequester();

//////////////////
// MAIN METHODS //
public: //////////

	// Set to use multiple thread queues
	bool UseThreadedQueue(uint32_t _totalNumberMaximumThreads, std::function<uint32_t()> _threadIndexMethod);

	// Request a file
	PacketFile* RequestFile(PacketFragment::FileIdentifier _fileIdentifier, PacketFile::DispatchType _dispatchType = PacketFile::DispatchType::OnProcess, bool _delayAllocation = false);

	// Process all loading queues (this requires synchronization and isn't thread safe)
	bool ProcessLoadingQueues();

private:

	// The process request method
	bool ProcessRequest(FileRequestData _requestData);

///////////////
// VARIABLES //
private: //////

	// If we are using the thread queue mode (we will use multiples arrays
	// (one for each thread) to allow multiple simultaneous access without mutex uses)
	bool m_UseThreadQueue;

	// The maximum of total threaded queues
	uint32_t m_MaximumTotalThreadedQueues;

	// Our packet file loader
	PacketFileLoader m_PacketFileLoader;

	// The threaded index method <used to retrieve the current running thread index>
	std::function<uint32_t()> m_ThreadIndexMethod;

	//
	
	// The base file request queue
	std::vector<FileRequestData> m_BaseRequestQueue; // TODO usar algum vetor com memory allocation diferente? Setar size inicial? Fazer com que ele nunca recue no size?

	// The threaded queues
	std::vector<FileRequestData>* m_ThreadedRequestQueues; // TODO usar algum vetor com memory allocation diferente? Setar size inicial? Fazer com que ele nunca recue no size?
};

// Packet data explorer
PacketNamespaceEnd(Packet)