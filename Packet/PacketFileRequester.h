////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileRequester.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketError.h"
#include "PacketMultipleQueue.h"
#include "PacketFileLoader.h"
#include "PacketFile.h"
#include "PacketFileReference.h"
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
		// The file reference
		PacketFileReference* fileReference;

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
	PacketFileRequester(PacketFileLoader& _fileLoaderReference, PacketFileStorage& _fileStorageReference);
	~PacketFileRequester();

//////////////////
// MAIN METHODS //
public: //////////

	// Set to use multiple thread queues
	bool UseThreadedQueue(uint32_t _totalNumberMaximumThreads, std::function<uint32_t()> _threadIndexMethod);

	// Request a file
	// TODO usar template com argumento do tipo PacketFile?
	bool RequestFile(PacketFileReference* _fileReference, PacketFragment::FileIdentifier _fileIdentifier, PacketFile::DispatchType _dispatchType = PacketFile::DispatchType::OnProcess, bool _delayAllocation = false);

	// Process all file queues (this requires synchronization and isn't thread safe)
	void ProcessFileQueues();

private:

	// The process request method
	bool ProcessRequest(FileRequestData _requestData);

///////////////
// VARIABLES //
private: //////

	// Our packet file loader and storage references
	PacketFileLoader& m_FileLoaderReference;
	PacketFileStorage& m_FileStorageReference;

	// Our request queue
	Packet::MultipleQueue<FileRequestData> m_RequestQueue;
};

// Packet data explorer
PacketNamespaceEnd(Packet)