////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceFactory.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "../../PacketConfig.h"

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

	// Request a new object
	virtual std::unique_ptr<PacketResource> RequestObject(const std::vector<uint8_t>& _resource_data) = 0;
	
	// Release an object
	virtual void ReleaseObject(std::unique_ptr<PacketResource> _object) = 0;

///////////////
// VARIABLES //
private: //////

};

// Packet
PacketDevelopmentNamespaceEnd(Packet)
