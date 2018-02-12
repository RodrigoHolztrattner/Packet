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
#include "PacketFileStorage.h"
#include "PacketFileRemover.h"

#include <string>
#include <functional>
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
// Class name: PacketFileRequester
////////////////////////////////////////////////////////////////////////////////
class PacketFileRequester
{

private:

	// Our file request type
	struct FileRequestData
	{
		FileRequestData(FutureReference<PacketFile>& _fileReference) : fileReference(_fileReference) {}

		// The file reference
		FutureReference<PacketFile>& fileReference;

		// The file identifier
		PacketFragment::FileIdentifier fileIdentifier;

		// The dispatch type
		PacketFile::DispatchType fileDispatchType;

		// If the memory allocation should be delayed
		bool delayAllocation;
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
	bool RequestFile(FutureReference<PacketFile>& _futureObject, PacketFragment::FileIdentifier _fileIdentifier, PacketFile::DispatchType _dispatchType = PacketFile::DispatchType::OnProcess, bool _delayAllocation = false);
	bool RequestFile(FutureReference<PacketFile>& _futureObject, const char* _fileName, PacketFile::DispatchType _dispatchType = PacketFile::DispatchType::OnProcess, bool _delayAllocation = false);

	// Process all file queues (this requires synchronization and isn't thread safe)
	bool ProcessFileQueues();

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

	// Our packet file loader, storage and remover
	PacketFileLoader m_PacketFileLoader;
	PacketFileStorage m_PacketFileStorage;
	PacketFileRemover m_PacketFileRemover;

	// The threaded index method <used to retrieve the current running thread index>
	std::function<uint32_t()> m_ThreadIndexMethod;

	// The mutex we will use to secure thread safe
	std::mutex m_Mutex;
	
	// The base file request queue
	std::vector<FileRequestData> m_BaseRequestQueue; // TODO usar algum vetor com memory allocation diferente? Setar size inicial? Fazer com que ele nunca recue no size?

	// The threaded queues
	std::vector<FileRequestData>* m_ThreadedRequestQueues; // TODO usar algum vetor com memory allocation diferente? Setar size inicial? Fazer com que ele nunca recue no size?
};

// Packet data explorer
PacketNamespaceEnd(Packet)