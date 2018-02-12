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

// We know the PacketObject, the PacketFileLoader, the PacketFileRequester, the PacketFileStorage and the PacketFileRemover classes
class PacketObject;
class PacketFileLoader;
class PacketFileRemover;
class PacketFileRequester;
class PacketFileStorage;

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFile
////////////////////////////////////////////////////////////////////////////////
class PacketFile
{
public:

	// The PacketFileLoader, the PacketFileRemover, the PacketFileStorage and the PacketFileRequester are friend classes
	friend PacketFileLoader;
	friend PacketFileRemover;
	friend PacketFileRequester;
	friend PacketFileStorage;

	// The dispatch type
	enum class DispatchType
	{
		OnRequest,
		OnProcess,
		Assync
	};

//////////////////
// CONSTRUCTORS //
protected: ///////

	// Constructor / destructor
	PacketFile(PacketFileRemover* _fileRemoverReference, PacketFragment::FileIdentifier _fileIdentifier, DispatchType _dispatchType = DispatchType::OnProcess, bool _delayAllocation = false);
	~PacketFile();

//////////////////
// MAIN METHODS //
public: //////////

	// Set the load callback (the callback will be fired when the loading phase ends)
	void SetLoadCallback(std::function<void()> _loadCallback);

	// Return the file dispatch type
	DispatchType GetDispatchType();

	// Release this file object
	void Release();

	// If the memory allocation should be delayed (only allocate when the loading starts)
	bool AllocationIsDelayed();

	// Return if this file is ready
	bool IsReady();

	// Return if this file is dirty
	bool IsDirty();

	// Return if this file was released
	bool WasReleased();

	// Return if there is an error on this file
	bool HasError();

	// Return the file identifier
	PacketFragment::FileIdentifier GetFileIdentifier();

	// Return the reference count
	uint32_t GetReferenceCount();

protected:

	// Allocate this file data
	bool AllocateData(uint32_t _fileSize);

	// Return a ptr to the internal data
	unsigned char* GetInternalDataPtr();

	// Finish the loading for this file (called from the packet file loader object)
	void FinishLoading();

	// Increment the reference count (called by the packet file storage object)
	void IncrementReferenceCount();

	// Decrement the reference count (called by the packet file storage object)
	void DecrementReferenceCount();

private:

	// Virtual method for memory allocation and deallocation
	virtual unsigned char* AllocateMemory(uint32_t _fileSize);
	virtual void DeallocateMemory(unsigned char* _fileData);

///////////////
// VARIABLES //
private: //////

	// The total number of references <only used by the file requester>
	uint32_t m_TotalNumberReferences;

	// If this file is ready (if it was filled with data)
	//  If this file is dirty (if the metadata info is invalid)
	//   If the allocation should be delayed
	bool m_IsReady;
	bool m_IsDirty;
	bool m_WasReleased;
	bool m_DelayAllocation;

	// The file identifier
	PacketFragment::FileIdentifier m_FileIdentifier;

	// The dispatch type
	DispatchType m_DispatchType;

	// The file remover reference
	PacketFileRemover* m_FileRemoverRefernce;

	// The load callback
	std::function<void()> m_LoadCallback;

	// The file data
	unsigned char* m_Data;

	// The error object
	PacketError m_ErrorObject; // TODO check if this is necessary
};

// Packet data explorer
PacketNamespaceEnd(Packet)