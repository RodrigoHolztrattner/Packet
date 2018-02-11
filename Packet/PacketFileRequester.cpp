////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileRequester.h"
#include "PacketObject.h"
#include "PacketFileLoader.h"

Packet::PacketFileRequester::PacketFileRequester(PacketObject* _packetObject)
{
	// Set our initial data
	m_PacketObjectReference = _packetObject;
}

Packet::PacketFileRequester::~PacketFileRequester()
{
}