////////////////////////////////////////////////////////////////////////////////
// Filename: PacketEditModeFileLoader.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketScanner.h"
#include "PacketCondenser.h"

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

////////////////
// FORWARDING //
////////////////

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketEditModeFileLoader
////////////////////////////////////////////////////////////////////////////////
class PacketEditModeFileLoader : public PacketFileLoader
{
public:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketEditModeFileLoader(std::string _packetManifestDirectory, PacketLogger* _logger);
	~PacketEditModeFileLoader();

//////////////////
// MAIN METHODS //
public: //////////

	// Check if a given file exist
	bool FileExist(Hash _fileHash) const override;

	// Return a file size
	uint64_t GetFileSize(Hash _fileHash) const override;

	// Get the file data
	bool GetFileData(uint8_t* _dataOut, uint64_t _bufferSize, Hash _fileHash) const override;

	// Pack all files
	bool ConstructPacket() override;

///////////////
// VARIABLES //
private: //////

	// The helper objects
	PacketScanner m_Scanner;
	PacketCondenser m_PacketCondenser;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
