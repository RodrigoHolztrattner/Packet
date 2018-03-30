////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileReference.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileReference.h"
#include "PacketObject.h"

Packet::PacketFileReference::PacketFileReference()
{
	// Set our initial data
	m_PacketFileReference = nullptr;
	m_IsReady = false;
	m_WasReleased = false;
}

Packet::PacketFileReference::PacketFileReference(std::function<void()> _setCallback)
{
	// Set our initial data
	m_PacketFileReference = nullptr;
	m_IsReady = false;
	m_WasReleased = false;
	m_ReadyCallback = _setCallback;
}

Packet::PacketFileReference::~PacketFileReference()
{
	// Check if this reference was released
	if (!m_WasReleased)
	{
		// Ops, we have a problem
		// TODO
	}
}

bool Packet::PacketFileReference::IsReady()
{
	return m_IsReady;
}

bool Packet::PacketFileReference::WasReleased()
{
	return m_WasReleased;
}

Packet::PacketFile* Packet::PacketFileReference::GetFileObject()
{
	return m_PacketFileReference;
}

void Packet::PacketFileReference::Release()
{
	// Set ready to false
	m_IsReady = false;
	m_WasReleased = true;
}
	
void Packet::PacketFileReference::SetFileReference(PacketFile* _fileReference)
{
	m_PacketFileReference = _fileReference;
}

void Packet::PacketFileReference::CallReadyCallback()
{
	// Set we are ready
	m_IsReady = true;

	// If we have a ready callback...
	if (m_ReadyCallback)
	{
		// Call the ready callback
		m_ReadyCallback();
	}
}