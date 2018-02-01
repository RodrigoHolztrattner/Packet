////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketObjectHashTable.h"
#include "PacketFileDataOperations.h"

Packet::PacketObjectHashTable::PacketObjectHashTable()
{
// Set the initial data
// ...
}

Packet::PacketObjectHashTable::~PacketObjectHashTable()
{
}

bool Packet::PacketObjectHashTable::InitializeEmpty()
{
	// Do nothing
	// ...

	return true;
}

bool Packet::PacketObjectHashTable::InitializeFromData(std::vector<unsigned char>& _data, uint32_t& _location)
{
	// Get the total number of entries
	uint32_t totalEntries = PacketFileDataOperations::ReadFromData<uint32_t>(_data, _location);

	// For each entry
	for (unsigned int i = 0; i < totalEntries; i++)
	{
		// Get the key
		PacketObjectHash key = PacketFileDataOperations::ReadFromData<PacketObjectHash>(_data, _location);

		// Get the file fragment identifier
		PacketObjectManager::FileFragmentIdentifier fileFragmentIdentifier = PacketFileDataOperations::ReadFromData<PacketObjectManager::FileFragmentIdentifier>(_data, _location);

		// Insert a new entry
		if (!InsertEntry(key, fileFragmentIdentifier, key))
		{
			return false;
		}
	}

	return true;
}

std::vector<unsigned char> Packet::PacketObjectHashTable::Serialize()
{
	// The data
	std::vector<unsigned char> data;
	uint32_t location = 0;

	// Save the total number of entries
	PacketFileDataOperations::SaveToData<uint32_t>(data, location, (uint32_t)m_HashTable.size());

	// For each map entry
	for (std::map<PacketObjectHash, PacketObjectManager::FileFragmentIdentifier>::iterator it = m_HashTable.begin(); it != m_HashTable.end(); ++it)
	{
		// Get the key
		PacketObjectHash internalKey = it->first;

		// Get the file fragment identifier
		PacketObjectManager::FileFragmentIdentifier fileFragmentIdentifier = it->second;

		// Save the key
		PacketFileDataOperations::SaveToData<PacketObjectHash>(data, location, internalKey);

		// Save the vector size
		PacketFileDataOperations::SaveToData<PacketObjectManager::FileFragmentIdentifier>(data, location, fileFragmentIdentifier);
	}

	return data;
}

bool Packet::PacketObjectHashTable::InsertEntry(std::string _path, PacketObjectManager::FileFragmentIdentifier& _internalIdentifier, PacketObjectHash& _hashOut)
{
	// Hash the path
	PacketObjectHash hash = HashFilePathNonStatic(_path.c_str());

	return InsertEntry(hash, _internalIdentifier, _hashOut);
}

bool Packet::PacketObjectHashTable::InsertEntry(PacketObjectHash _key, PacketObjectManager::FileFragmentIdentifier& _internalIdentifier, PacketObjectHash& _hashOut)
{
	// Find the entry
	auto it = m_HashTable.find(_key);
	if (it == m_HashTable.end())
	{
		// Insert it
		m_HashTable.insert(std::pair<PacketObjectHash, PacketObjectManager::FileFragmentIdentifier>(_key, _internalIdentifier));

		// Set the hash
		_hashOut = _key;

		return true;
	}

	return false;
}

bool Packet::PacketObjectHashTable::RemoveEntry(std::string _path)
{
	// Hash the path
	PacketObjectHash hash = HashFilePathNonStatic(_path.c_str());

	// Find the entry
	auto it = m_HashTable.find(hash);
	if (it == m_HashTable.end())
	{
		// We cant find this entry
		return false;
	}

	// Remove this entry
	m_HashTable.erase(it);

	return true;
}

bool Packet::PacketObjectHashTable::EntryExist(std::string _path)
{
	// Hash the path
	PacketObjectHash hash = HashFilePathNonStatic(_path.c_str());

	// Find the entry
	auto it = m_HashTable.find(hash);
	if (it == m_HashTable.end())
	{
		// We cant find this entry
		return false;
	}

	return true;
}

std::vector<Packet::PacketObjectManager::FileFragmentIdentifier> Packet::PacketObjectHashTable::GetHashAsVector()
{
	std::vector<Packet::PacketObjectManager::FileFragmentIdentifier> result;

	// Move each entry to the vector
	for (auto it = m_HashTable.begin(); it != m_HashTable.end(); ++it) 
	{
		result.push_back(it->second);
	}

	return result;
}

bool Packet::PacketObjectHashTable::UpdateHashWithVector(std::vector<PacketObjectManager::FileFragmentIdentifier> _hashVector)
{
	// Check if the size match
	if (m_HashTable.size() != _hashVector.size())
	{
		return false;
	}

	// For each actual entry
	int index = 0;
	for (auto& it = m_HashTable.begin(); it != m_HashTable.end(); ++it)
	{
		// Update the entry
		it->second = _hashVector[index];

		// Increment the index
		index++;
	}

	return true;
}

Packet::PacketObjectManager::FileFragmentIdentifier* Packet::PacketObjectHashTable::GetEntry(std::string _path)
{
	// Hash the path
	PacketObjectHash hash = HashFilePathNonStatic(_path.c_str());

	return GetEntry(hash);
}

Packet::PacketObjectManager::FileFragmentIdentifier* Packet::PacketObjectHashTable::GetEntry(PacketObjectHash _hash)
{
	// Find the entry
	auto it = m_HashTable.find(_hash);
	if (it == m_HashTable.end())
	{
		// We cant find this entry
		return nullptr;
	}

	// Return the entry
	return &it->second;
}