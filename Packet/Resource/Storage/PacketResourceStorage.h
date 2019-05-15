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
	PacketResourceStorage(OperationMode _operation_mode);
	~PacketResourceStorage();

//////////////////
// MAIN METHODS //
public: //////////

	// Try to find an object with the input hash, if successful, return it
	PacketResource* FindObject(Hash _hash, uint32_t _buildFlags) const;

    // Retrieve all resource objects associated with the given hash, this method is only available on
    // Plain mode and it will return an empty set if called on Condensed mode
    // I could return a const reference or a pointer but since this will rarely be called I will prioritize
    // readability instead performance, actually the compiler will probably optimize and use a reference since
    // the returned set won't be modified
    std::set<PacketResource*> GetAllObjectsWithHash(HashPrimitive _hash_primitive) const;

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

    // Our current operation mode
    OperationMode m_OperationMode;

	// The object map
	std::map<std::pair<HashPrimitive, uint32_t>, std::unique_ptr<PacketResource>> m_ObjectMap;

    // This map will only be filled if the current mode isn't the Condensed one, it will be
    // used as a helper object when querying directly by a HashPrimitive is necessary and
    // we must return all loaded resources that are tied to that hash
    std::map<HashPrimitive, std::set<PacketResource*>> m_HashLinkedResourcesMap;
};

// Packet
PacketDevelopmentNamespaceEnd(Packet)