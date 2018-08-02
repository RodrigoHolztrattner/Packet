////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceFactory.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "..\PacketConfig.h"

#include <cstdint>
#include <vector>
#include <atomic>

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
class PacketResourceManager;
class PacketResourceInstance;
struct PacketResourceData;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketResourceFactory
////////////////////////////////////////////////////////////////////////////////
class PacketResourceFactory
{
public:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketResourceFactory();
	virtual ~PacketResourceFactory();

//////////////////
// MAIN METHODS //
public: //////////

	// Request a new instance
	virtual std::unique_ptr<PacketResourceInstance> RequestInstance(Hash _hash, PacketResourceManager* _resourceManager) = 0;

	// Release a instance
	virtual void ReleaseInstance(std::unique_ptr<PacketResourceInstance>& _instance) = 0;

	// Request a new object
	virtual std::unique_ptr<PacketResource> RequestObject() = 0;
	
	// Release an object // TODO make this method callable only for the manager (sync, not async (the way it currently works right now))
	virtual void ReleaseObject(std::unique_ptr<PacketResource>& _object) = 0;

	// Allocates the given amount of data for the resource creation
	virtual bool AllocateData(PacketResourceData& _resourceDataRef, uint64_t _total) = 0;

	// Deallocates the given data from the resource
	virtual void DeallocateData(PacketResourceData& _data) = 0;

///////////////
// VARIABLES //
private: //////

};

// Packet
PacketDevelopmentNamespaceEnd(Packet)