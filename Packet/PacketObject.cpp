////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketObject.h"

Packet::PacketObject::PacketObject()
{
	// Set the initial data
	// ...
}

Packet::PacketObject::~PacketObject()
{
}

Packet::PacketObjectIterator Packet::PacketObject::GetIterator()
{
	PacketObjectManager::PacketAttributes attributes;
	attributes.packetObjectName = "Wonderland";
	attributes.maximumFragmentSize = 8096;
	m_ObjectManager.SetPacketAttributes(attributes);
	return PacketObjectIterator(m_ObjectManager, m_ObjectStructure, m_ObjectHashTable);
}