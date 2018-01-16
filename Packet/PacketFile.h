////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFile.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketFragment.h"
#include "PacketObjectManager.h"
#include "PacketError.h"

#include <string>

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

// We know the PacketObject and the PacketFileLoader classes
class PacketObject;
class PacketFileLoader;

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFile
////////////////////////////////////////////////////////////////////////////////
class PacketFile
{
public:

	// The PacketFileLoader is a friend class
	friend PacketFileLoader;

	// The dispatch type
	enum class DispatchType
	{
		Sync,
		Assync
	};

	// The metadata type
	struct Metadata
	{
		// The file identifier
		PacketFragment::FileIdentifier fileIdentifier;

		// The file fragment identifier
		PacketObjectManager::FileFragmentIdentifier fileFragmentIdentifier;

		// The file size
		unsigned int fileSize;
	};

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFile(PacketObject* _packetObject, DispatchType _dispatchType = DispatchType::Sync, bool _delayAllocation = false);
	~PacketFile();

//////////////////
// MAIN METHODS //
public: //////////

	// Load the file with the given identifier/name
	bool LoadWithIdentifier(PacketFragment::FileIdentifier _fileIdentifier);
	bool LoadWithName(const char* _fileName);

	// Set the load callback (the callback will be fired when the loading phase ends)
	void SetLoadCallback(std::function<void()> _loadCallback);

	// Return this file metadata <only valid after the loading phase begin>
	Metadata GetMetadata();

	// Return the file dispatch type
	DispatchType GetDispatchType();

	// If the memory allocation should be delayed (only allocate when the loading starts)
	bool AllocationIsDelayed();

	// Return if this file is ready
	bool IsReady();

	// Return if this file is dirty
	bool IsDirty();

protected:

	// Allocate this file data
	bool AllocateData();

	// Return the file identifier
	PacketFragment::FileIdentifier GetFileIdentifier();

	// Return a ptr to the internal data
	unsigned char* GetInternalDataPtr();

	// Finish the loading for this file (called from the packet file loader object)
	void FinishLoading();

private:

	// Release?


private:

	// Virtual method for memory allocation and deallocation
	virtual unsigned char* AllocateMemory(unsigned int _fileSize);
	virtual void DeallocateMemory(unsigned char* _fileData);

///////////////
// VARIABLES //
private: //////

	// Our PacketObject reference
	PacketObject* m_PacketObjectReference;

	// If this file is ready (if it was filled with data)
	bool m_IsReady;

	// If this file is dirty (if the metadata info is invalid)
	bool m_IsDirty;

	// If the allocation should be delayed
	bool m_DelayAllocation;

	// The dispatch type and metadata info
	DispatchType m_DispatchType;
	Metadata m_Metadata;

	// The load callback
	std::function<void()> m_LoadCallback;

	// The file data
	unsigned char* m_Data;

	// The error object
	PacketError m_ErrorObject;
};

// Packet data explorer
PacketNamespaceEnd(Packet)