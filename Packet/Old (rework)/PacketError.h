////////////////////////////////////////////////////////////////////////////////
// Filename: PacketError.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"

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
PacketDevelopmentNamespaceBegin(Packet)

/////////////
// DEFINES //
/////////////
#define PacketErrorVerbose

///////////////////
// ERROR DEFINES //
///////////////////

// No error
#define PacketErrorNoError					0

// Iterator errors
#define PacketErrorPathNotDirectory			501
#define PacketErrorPathNotFile				502
#define PacketErrorInvalidDirectory			503
#define PacketErrorInvalidFile				504
#define PacketErrorHashDuplicate			505
#define PacketErrorFileFromPathInvalid		506
#define PacketErrorFileFromPathDuplicated	507
#define PacketErrorInvalidFileData			508
#define PacketErrorStructureInsert			509
#define PacketErrorRetrieveData				510
#define PacketErrorFolderCreationFailed		511
#define PacketErrorDeleteFile				512
#define PacketErrorDeleteHashEntry			513
#define PacketErrorDeleteStructureEntry		514

// PacketFile errors
#define PacketErrorInvalidFileIdentifier	1101
#define PacketErrorFileDataAlreadyAllocated	1102

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketError
////////////////////////////////////////////////////////////////////////////////
class PacketError
{
//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketError();
	~PacketError();

	// Set the error (also clears it)
	void Set(uint32_t _errorIdentifier);
	void Set(uint32_t _errorIdentifier, std::string _errorMessage);

	// Get the error info
	bool GetInfo(uint32_t& _errorIdentifier, std::string& _errorMessage, bool _clearCurrentError = true);

	// Print the current error info (terminal)
	bool PrintInfo(bool _clearCurrentError = true);

	// If there is an error
	bool IsSet();

	// If there is an error, clear it
	void Clear();

/////////////
// STRINGS //
public: /////

	// The error id
	uint32_t m_ErrorIdentifier;

	// The error message
	std::string m_ErrorMessage;

	// If this error was set
	bool m_IsSet;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)