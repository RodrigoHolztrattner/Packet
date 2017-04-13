////////////////////////////////////////////////////////////////////////////////
// Filename: PacketIndexLoader.h
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
// Class name: PacketIndexLoader
////////////////////////////////////////////////////////////////////////////////
class PacketIndexLoader
{
public:

	const std::string IndexExtension = ".index";

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketIndexLoader();
	~PacketIndexLoader();

//////////////////
// MAIN METHODS //
public: //////////

	// Initialize
	bool Initialize(std::string _rootFileName, PacketDirectory* _rootDirectory, PacketIndex& _indexData);

	// Release
	void Release();

	// Save the packet index data
	void SaveIndex(PacketIndex& _indexData, PacketDirectory* _rootDirectory);


private:

	// Load the packet index data
	void LoadIndex(PacketIndex& _indexData, PacketDirectory* _rootDirectory);

	// Create the base index file
	void CreateIndexFile(PacketDirectory* _rootDirectory);

	// Read a folder from the given file
	void ReadFolder(std::vector<unsigned char>& _data, uint32_t& _index, PacketDirectory* _folder, PacketIndex& _indexData);

	// Read a file info from the file to the given folder
	void ReadFile(std::vector<unsigned char>& _data, uint32_t& _index, PacketFile* _fileInfo, PacketIndex& _indexData);

	// Write a folder
	void WriteFolder(std::vector<unsigned char>& _data, PacketDirectory* _folder);

	// Write a file
	void WriteFile(std::vector<unsigned char>& _data, PacketFile* _fileInfo);

private:

	// Read from the given file
	template <typename ObjectType>
	bool ReadFromFile(std::ifstream& _file, ObjectType& _object, uint32_t _amount = 1)
	{
		_file.read((char*)&_object, sizeof(ObjectType) * _amount); return true;
	}

	// Read a stream from the file
	bool ReadStreamFromFile(std::ifstream& _file, void* _object, uint32_t _unitSize, uint32_t _amount = 1)
	{
		_file.read((char*)_object, _unitSize * _amount); return true;
	}

	// Write into the given file
	template <typename ObjectType>
	bool WriteToFile(std::ofstream& _file, ObjectType& _object, uint32_t _amount = 1)
	{
		_file.write((char*)&_object, sizeof(ObjectType) * _amount); return true;
	}

	// Write a stream into the given file
	bool WriteStreamToFile(std::ofstream& _file, void* _object, uint32_t _unitSize, uint32_t _amount = 1)
	{
		_file.write((char*)_object, _unitSize * _amount); return true;
	}

///////////////
// VARIABLES //
private: //////

	// The root file name
	std::string m_RootFileName;

	// The index file name
	std::string m_IndexFileName;
};

// Packet data explorer
NamespaceEnd(Packet)