////////////////////////////////////////////////////////////////////////////////
// Filename: PacketLoader.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>

#include "PacketMode.h"
#include "PacketString.h"
#include "PacketFile.h"
#include "PacketDirectory.h"
#include "PacketIndex.h"

#include "..\NamespaceDefinitions.h"

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
NamespaceBegin(Packet)

////////////////
// FORWARDING //
////////////////

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketLoader
////////////////////////////////////////////////////////////////////////////////
class PacketLoader
{
public:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketLoader();
	~PacketLoader();

//////////////////
// MAIN METHODS //
public: //////////

	// Load a file
	bool LoadFile(PacketFile* _fileInfo, std::vector<unsigned char>& _byteArray);

	// Save a file into the vault
	bool SaveFileIntoVault(PacketFile* _fileInfo, std::vector<unsigned char>& _byteArray);

protected:

	// Change the operation mode
	void ChangeLoaderOperationMode(PacketMode _mode) { m_OperationMode = _mode; }

private:

	// Load a file from raw data
	bool LoadRawFile(PacketFile* _fileInfo, std::vector<unsigned char>& _byteArray);

///////////////
// VARIABLES //
private: //////

	// The current operation mode
	PacketMode m_OperationMode;
};

// Packet data explorer
NamespaceEnd(Packet)