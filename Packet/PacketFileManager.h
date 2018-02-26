////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileManager.h
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
#include "PacketFileRequester.h"
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
// Class name: PacketFileManager
////////////////////////////////////////////////////////////////////////////////
class PacketFileManager
{

private:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFileManager(PacketObject* _packetObject);
	~PacketFileManager();

//////////////////
// MAIN METHODS //
public: //////////

	// Set to use multiple thread queues
	bool UseThreadedQueue(uint32_t _totalNumberMaximumThreads, std::function<uint32_t()> _threadIndexMethod);

	// Request a file reference
	bool RequestReference(PacketFileReference* _fileReference, PacketFragment::FileIdentifier _fileIdentifier, PacketFile::DispatchType _dispatchType = PacketFile::DispatchType::OnProcess, bool _delayAllocation = false);
	bool RequestReference(PacketFileReference* _fileReference, const char* _fileName, PacketFile::DispatchType _dispatchType = PacketFile::DispatchType::OnProcess, bool _delayAllocation = false);

	// Release a file reference
	bool ReleaseReference(PacketFileReference* _fileReference);

	// Process all file queues (this requires synchronization and isn't thread safe)
	void ProcessQueues();

private:

///////////////
// VARIABLES //
private: //////

	// Our packet file requester, loader, storage and remover
	PacketFileLoader m_PacketFileLoader;
	PacketFileStorage m_PacketFileStorage;
	PacketFileRequester m_PacketFileRequester;
	PacketFileRemover m_PacketFileRemover;
};

// Packet data explorer
PacketNamespaceEnd(Packet)