////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileReference.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketFile.h"
#include "PacketError.h"

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

// The classes we know
class PacketFileRequester;
class PacketFileManager;

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFileReference
////////////////////////////////////////////////////////////////////////////////
class PacketFileReference
{
	// Our friend classes
	friend PacketFileRequester;
	friend PacketFile;
	friend PacketFileManager;

public:

//////////////////
// CONSTRUCTORS //
protected: ///////

	// Constructor / destructor
	PacketFileReference();
	~PacketFileReference();

//////////////////
// MAIN METHODS //
public: //////////

	// Return if this reference is ready to be used
	bool IsReady();

	// Return if this reference was released
	bool WasReleased();

	// Return a ptr to the file object
	PacketFile* GetFileObject();

	// Set the ready callback function
	void SetReadyCallback(std::function<void()> _readyCallback);

protected:

	// Release this reference
	void Release();

	// Set file reference
	void SetFileReference(PacketFile* _fileReference);

	// Call the ready callback
	void CallReadyCallback();

///////////////
// VARIABLES //
private: //////

	// The packet file reference
	PacketFile* m_PacketFileReference;

	// The ready callback
	std::function<void()> m_ReadyCallback;

	// If this file reference is ready to be used
	bool m_IsReady;

	// If this file reference was released
	bool m_WasReleased;
};

// Packet data explorer
PacketNamespaceEnd(Packet)