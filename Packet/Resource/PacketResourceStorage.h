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

	// Try to find an object with the input hash, if successful, return it
	PacketResource* FindObject(Hash _hash, uint32_t _buildFlags) const;

	// Insert a new object
	bool InsertObject(std::unique_ptr<PacketResource> _object, Hash _hash, uint32_t _buildFlags);

	// Replace an resource by a new one
	std::unique_ptr<PacketResource> ReplaceObject(std::unique_ptr<PacketResource>& _object, Hash _hash, uint32_t _buildFlags);

	// Remove an object
	bool RemoveObject(PacketResource* _object);
	bool RemoveObject(Hash _hash, uint32_t _buildFlags);

	// Remove an object returning its ownership
	std::unique_ptr<PacketResource> GetObjectOwnership(PacketResource* _object);

	// Return a reference to our object map
    const std::map<std::pair<HashPrimitive, uint32_t>, std::unique_ptr<PacketResource>>& GetObjectMapReference() const;

///////////////
// VARIABLES //
private: //////

	// The object map
	std::map<std::pair<HashPrimitive, uint32_t>, std::unique_ptr<PacketResource>> m_ObjectMap;
};

// Packet
PacketDevelopmentNamespaceEnd(Packet)