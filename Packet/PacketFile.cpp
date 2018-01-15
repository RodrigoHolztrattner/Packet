////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFile.h"
#include "PacketObject.h"
#include "PacketFileLoader.h"

Packet::PacketFile::PacketFile(PacketObject* _packetObject, DispatchType _dispatchType, bool _delayAllocation)
{
	// Set our initial data
	m_PacketObjectReference = _packetObject;
	m_DispatchType = _dispatchType;
	m_DelayAllocation = _delayAllocation;
	m_IsReady = false;
	m_IsDirty = true;
}

Packet::PacketFile::~PacketFile()
{
}

bool Packet::PacketFile::LoadWithIdentifier(PacketFragment::FileIdentifier _fileIdentifier)
{
	// Get the packet file loader instance
	PacketFileLoader* packetFileLoader = m_PacketObjectReference->GetFileLoader();

	// Set the metadata
	m_Metadata.fileIdentifier = _fileIdentifier;
	m_Metadata.fileFragmentIdentifier = packetFileLoader->GetFileFragmentIdentifier(_fileIdentifier, m_PacketObjectReference);
	m_Metadata.fileSize = packetFileLoader->GetFileSize(m_Metadata.fileFragmentIdentifier, m_PacketObjectReference);

	// Set dirty to false (we should use a barrier here to prevent random ordering?)
	m_IsDirty = false;

	// Process this packet file
	return packetFileLoader->ProcessPacketFile(this, m_PacketObjectReference);
}

void Packet::PacketFile::SetLoadCallback()
{

}

Packet::PacketFile::Metadata Packet::PacketFile::GetMetadata()
{
	return m_Metadata;
}

Packet::PacketFile::DispatchType Packet::PacketFile::GetDispatchType()
{
	return m_DispatchType;
}

bool Packet::PacketFile::IsReady()
{
	return m_IsReady;
}

bool Packet::PacketFile::IsDirty()
{
	return m_IsDirty;
}

Packet::PacketFragment::FileIdentifier Packet::PacketFile::GetFileIdentifier()
{
	return m_Metadata.fileIdentifier;
}

unsigned char* Packet::PacketFile::AllocateMemory(unsigned int _fileSize)
{
	return new unsigned char[_fileSize];
}

void Packet::PacketFile::DeallocateMemory(unsigned char* _fileData)
{
	delete[] _fileData;
}