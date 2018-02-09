////////////////////////////////////////////////////////////////////////////////
// Filename: PacketObjectManager.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketFragment.h"

#include <string>
#include <vector>

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

// We know the PacketObjectIterator class
class PacketObjectIterator;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketObjectManager
////////////////////////////////////////////////////////////////////////////////
class PacketObjectManager
{
private:

	// The PacketObjectIterator is a friend class
	friend PacketObjectIterator;

	// The fragment info
	struct FragmentInfo
	{
		// The fragment name
		std::string fragmentName;
	};

public:

	// The file fragment identifier type
	struct FileFragmentIdentifier
	{
		// The fragment index
		uint32_t fragmentIndex;

		// The file identifier inside the fragment
		PacketFragment::FileIdentifier fileIdentifier;
	};

public:

	// The packet attributes
	struct PacketAttributes
	{
		PacketAttributes() {}
		PacketAttributes(std::string _packetObjectName, uint32_t maximumFragmentSize) : packetObjectName(_packetObjectName), maximumFragmentSize(maximumFragmentSize) {}

		// The packet object name
		std::string packetObjectName;

		// The maximum fragment size (this cannot be changed after set and has a maximum limit of 4.294.967.295 bytes or 4.2gb)
		uint32_t maximumFragmentSize;
	};

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketObjectManager();
	~PacketObjectManager();

	// Initialize empty
	bool InitializeEmpty(std::string _packetName, uint32_t _maximumFragmentSize);

	// Initialize from data
	bool InitializeFromData(std::vector<unsigned char>& _data, uint32_t& _location, std::string _packetName, uint32_t _maximumFragmentSize);

	// Serialize
	std::vector<unsigned char> Serialize();

//////////////////
// MAIN METHODS //
public: //////////

	// Insert a file into the packet object
	bool InsertFile(std::string _filePathOrigin, FileFragmentIdentifier& _hashidentifier);

	// Insert a chunk of data
	bool InsertData(unsigned char* _data, uint32_t _size, FileFragmentIdentifier& _hashidentifier);

	// Get a file size from the packet object
	uint32_t GetFileSize(FileFragmentIdentifier _hashidentifier);

	// Get a file from the packet object
	bool GetFile(std::string _filePathDestination, FileFragmentIdentifier _hashidentifier);

	// Get a chunk of data
	bool GetData(unsigned char* _data, FileFragmentIdentifier _hashidentifier);

	// Remove a file from this packet object
	bool RemoveFile(FileFragmentIdentifier _hashIdentifier);

	// Criar uma função que recebe uma função lambda de parâmetro que usaremos quando na otmização um hash identifier trocar de estado
	// ... TODO

protected:

	// Optimize all objects
	bool OptimizeFragmentsUsingIdentifiers(std::vector<FileFragmentIdentifier> _allFileFragmentIdentifiers, std::vector<FileFragmentIdentifier>& _outputFileFragmentIdentifiers);

private:

	// Return a valid fragment object (creating one if necessary)
	PacketFragment* GetValidFragment(uint32_t& _fragmentIndex);
	PacketFragment* GetValidFragmentForSize(uint32_t _size, uint32_t& _fragmentIndex);

	// Get the fragment with the given index
	PacketFragment* GetFragmentWithIndex(uint32_t _index);

	// Create a new fragment object
	PacketFragment* CreateNewFragment();

///////////////
// VARIABLES //
private: //////

	// The packet object attributes
	PacketAttributes m_PacketObjectAttributes;

	// All the fragment infos
	std::vector<FragmentInfo> m_FragmentInfos;

	// All fragment objects
	std::vector<PacketFragment*> m_Fragments;
};

// Packet data explorer
PacketNamespaceEnd(Packet)
