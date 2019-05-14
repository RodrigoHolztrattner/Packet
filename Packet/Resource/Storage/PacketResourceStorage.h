////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceStorage.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "../../PacketConfig.h"

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
	PacketResource* FindObject(Hash _hash, uint32_t _buildFlags, bool _isRuntimeResource) const;

	// Insert a new object
	bool InsertObject(std::unique_ptr<PacketResource> _object, Hash _hash, uint32_t _buildFlags);

	// Replace an resource by a new one
	std::unique_ptr<PacketResource> ReplaceObject(std::unique_ptr<PacketResource>& _object, Hash _hash, uint32_t _buildFlags);

	// Remove an object, passing its ownership
	std::unique_ptr<PacketResource> GetObjectOwnership(PacketResource* _object);

    // Return a vector with all permanent resources, passing their ownership
    std::vector<std::unique_ptr<PacketResource>> GetPermanentResourcesOwnership();

    // Return an approximation of the current number of resources since some of them could be enqueued 
    // to be created or released
    uint32_t GetAproximatedResourceAmount() const;

///////////////
// VARIABLES //
private: //////

	// The object map
	std::map<std::pair<HashPrimitive, uint32_t>, std::unique_ptr<PacketResource>> m_ObjectMap;

    // The runtime object map
    std::map<PacketResource*, std::unique_ptr<PacketResource>> m_RuntimeObjectMap;
};

// Packet
PacketDevelopmentNamespaceEnd(Packet)