////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketObjectHashTable.h"

Packet::PacketObjectHashTable::PacketObjectHashTable()
{
// Set the initial data
// ...
}

Packet::PacketObjectHashTable::~PacketObjectHashTable()
{
}

uint32_t Packet::PacketObjectHashTable::InsertEntry(std::string _path, PacketObjectManager::FileFragmentIdentifier& _internalIdentifier)
{
	// Hash the path
	uint32_t hash = HashFilePathNonStatic(_path.c_str());

	// Create the new hash internal object
	HashInternal hashInternal;
	hashInternal.fullPath = _path;
	hashInternal.fragmentIdentifier = _internalIdentifier;

	// Find the entry
	auto it = m_HashTable.find(hash);
	if (it == m_HashTable.end())
	{
		// Create the new vector with the internal identifier
		std::vector<HashInternal> hashInternalVector;

		// Insert the new hash internal object
		hashInternalVector.push_back(hashInternal);

		// Insert it
		m_HashTable.insert(std::pair<uint32_t, std::vector<HashInternal>>(hash, hashInternalVector));

		return hash;
	}

	// Add the new hash internal object
	it->second.push_back(hashInternal);

	return hash;
}

bool Packet::PacketObjectHashTable::RemoveEntry(std::string _path)
{
	// Hash the path
	uint32_t hash = HashFilePathNonStatic(_path.c_str());

	// Find the entry
	auto it = m_HashTable.find(hash);
	if (it == m_HashTable.end())
	{
		// We cant find this entry
		return false;
	}

	// Check if this entry has multiple values
	if (it->second.size() <= 1)
	{
		// Remove this entry
		m_HashTable.erase(it);

		return true;
	}

	// For each value
	for (unsigned int i = 0; i < it->second.size(); i++)
	{
		// Get the current hash internal object
		HashInternal* currentHashInternal = &it->second[i];

		// Compare the names
		if (currentHashInternal->fullPath.compare(_path) == 0)
		{
			// Remove this entry
			it->second.erase(it->second.begin() + i);

			return true;
		}
	}

	return false;
}

Packet::PacketObjectManager::FileFragmentIdentifier* Packet::PacketObjectHashTable::GetEntry(std::string _path)
{
	// Hash the path
	uint32_t hash = HashFilePathNonStatic(_path.c_str());

	return GetEntry(_path, hash);
}

Packet::PacketObjectManager::FileFragmentIdentifier* Packet::PacketObjectHashTable::GetEntry(std::string _path, uint32_t _hash)
{
	// Find the entry
	auto it = m_HashTable.find(_hash);
	if (it == m_HashTable.end())
	{
		// We cant find this entry
		return nullptr;
	}

	// For each value
	for (unsigned int i = 0; i < it->second.size(); i++)
	{
		// Get the current hash internal object
		HashInternal* currentHashInternal = &it->second[i];

		// Compare the names
		if (currentHashInternal->fullPath.compare(_path) == 0)
		{
			// Return the fragment identifier
			return &it->second[0].fragmentIdentifier;
		}
	}

	return nullptr;
}