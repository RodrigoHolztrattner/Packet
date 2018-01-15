////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileLoader.h"
#include "PacketFile.h"
#include "PacketObject.h"
#include "PacketObjectManager.h"

Packet::PacketFileLoader::PacketFileLoader(PacketObject* _packetObject)
{
	// Set our initial data
	m_PacketObjectReference = _packetObject;
}

Packet::PacketFileLoader::~PacketFileLoader()
{
}

bool Packet::PacketFileLoader::ProcessPacketFile(PacketFile* _packetFile)
{
	// Get the file manager reference from our packet object
	PacketObjectManager* objectManagerReference = m_PacketObjectReference->GetObjectManagerReference();

	// Get the file metadata
	PacketFile::Metadata fileMetadata = _packetFile->GetMetadata();

	// Check the file dispatch type
	if (_packetFile->GetDispatchType() == PacketFile::DispatchType::Sync)
	{
		// Allocate the file data
		// ...

		// Load this file
		return LoadFile(_packetFile);
	}

	// Insert it into our load queue
	// ...

	return true;
}

unsigned int Packet::PacketFileLoader::GetFileSize(PacketObjectManager::FileFragmentIdentifier* _fragmentFileIdentifier)
{
	// Get the file manager reference from our packet object
	PacketObjectManager* objectManagerReference = m_PacketObjectReference->GetObjectManagerReference();

	return objectManagerReference->GetFileSize(*_fragmentFileIdentifier);
}

Packet::PacketObjectManager::FileFragmentIdentifier* Packet::PacketFileLoader::GetFileFragmentIdentifier(PacketFragment::FileIdentifier _fileIdentifier)
{
	// Get the file manager reference from our packet object
	PacketObjectManager* objectManagerReference = m_PacketObjectReference->GetObjectManagerReference();

	return m_PacketObjectReference->GetObjectHashTableReference()->GetEntry(_fileIdentifier);;
}

bool Packet::PacketFileLoader::LoadFile(PacketFile* _file)
{
	// Add mutex
	// ...

	// Load the file
	// ...

	return true;
}