////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceHolder.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResourceHolder.h"

#include <cassert>

// Using namespace Peasant
PacketUsingDevelopmentNamespace(Packet)

PacketResourceHolder::PacketResourceHolder(PacketResource* _object)
{
	// Set the initial data
	m_ReferenceObject = _object;
}

PacketResourceHolder::~PacketResourceHolder()
{
}