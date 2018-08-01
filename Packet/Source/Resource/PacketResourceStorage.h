////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceStorage.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "..\PacketConfig.h"

#include <cstdint>
#include <vector>
#include <atomic>
#include <map>

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

// Packet
PacketDevelopmentNamespaceBegin(Packet)

//////////////
// TYPEDEFS //
//////////////

////////////////
// FORWARDING //
////////////////

// Classes we know
class PacketResource;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketResourceStorage
////////////////////////////////////////////////////////////////////////////////
class PacketResourceStorage
{
public:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketResourceStorage();
	~PacketResourceStorage();

//////////////////
// MAIN METHODS //
public: //////////

	// Try to find an object with the input hash, if sucessfull, return it
	PacketResource* FindObject(Hash _hash);

	// Insert a new object
	bool InsertObject(std::unique_ptr<PacketResource>& _object, Hash _hash);

	// Replace an resource by a new one
	std::unique_ptr<PacketResource> ReplaceObject(std::unique_ptr<PacketResource>& _object, Hash _hash);

	// Remove an object
	bool RemoveObject(PacketResource* _object);
	bool RemoveObject(Hash _hash);

	// Remove an object returning its ownership
	std::unique_ptr<PacketResource> GetObjectOwnership(PacketResource* _object);

	// Return a reference to our object map
	std::map<HashPrimitive, std::unique_ptr<PacketResource>>& GetObjectMapReference();

///////////////
// VARIABLES //
private: //////

	// The object map
	std::map<HashPrimitive, std::unique_ptr<PacketResource>> m_ObjectMap;
};

// Packet
PacketDevelopmentNamespaceEnd(Packet)