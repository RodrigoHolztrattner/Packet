////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileStorage.h"
#include "PacketObject.h"
#include "PacketFileLoader.h"

Packet::PacketFileStorage::PacketFileStorage()
{
	// Set our initial data
	// ...
}

Packet::PacketFileStorage::~PacketFileStorage()
{
}

bool Packet::PacketFileStorage::InserFileWithIdentifier(PacketFragment::FileIdentifier _fileIdentifier, PacketFile* _file)
{
	// Not thread safe //
	
	// Check if a file with the given identifier exist
	auto iterator = m_FileStorage.find(_fileIdentifier);
	if (iterator != m_FileStorage.end())
	{
		return false;
	}

	// Insert the file with the given identifier
	m_FileStorage.insert(std::pair<PacketFragment::FileIdentifier, PacketFile*>(_fileIdentifier, _file));

	// Increment the reference count
	_file->IncrementReferenceCount();

	return true;
}

Packet::PacketFile* Packet::PacketFileStorage::RequestFileFromIdentifier(PacketFragment::FileIdentifier _fileIdentifier)
{
	// Not thread safe //
	
	// Check if a file with the given identifier exist
	auto iterator = m_FileStorage.find(_fileIdentifier);
	if (iterator == m_FileStorage.end())
	{
		return nullptr;
	}

	// Get the file reference variable
	PacketFile* file = iterator->second;

	// Increment the reference count
	file->IncrementReferenceCount();
	
	return file;
}

bool Packet::PacketFileStorage::ShutdownFileFromIdentifier(PacketFragment::FileIdentifier _fileIdentifier)
{
	// Not thread safe //
	
	// Check if a file with the given identifier exist
	auto iterator = m_FileStorage.find(_fileIdentifier);
	if (iterator == m_FileStorage.end())
	{
		return false;
	}

	// Get the file reference variable
	PacketFile* file = iterator->second;

	// Decrement the reference count
	file->DecrementReferenceCount();
	
	// Check if the file has at last one reference
	if (file->GetReferenceCount() == 0)
	{
		// Delete this file
		delete file; // TODO use another deallocation method
		
		// Remove the file from the storage map
		m_FileStorage.erase(iterator);
	}
	
	return true;
}