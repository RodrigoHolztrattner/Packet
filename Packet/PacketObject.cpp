////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketObject.h"


Packet::PacketObject::PacketObject(std::string _packetName, uint32_t _maximumFragmentSize) : 
	m_ObjectManager(PacketObjectManager::PacketAttributes(_packetName, _maximumFragmentSize)),
	m_ObjectStructure(_packetName),
	m_ObjectHashTable(_packetName)
{
	// Set the initial data
	// ...
}

Packet::PacketObject::~PacketObject()
{
}

Packet::PacketObjectIterator Packet::PacketObject::GetIterator()
{
	return PacketObjectIterator(m_ObjectManager, m_ObjectStructure, m_ObjectHashTable);
}
