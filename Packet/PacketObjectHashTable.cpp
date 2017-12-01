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
	for (int i = 0; i < totalEntries; i++)
	{
		// Get the key
		uint32_t key = PacketFileDataOperations::ReadFromData<uint32_t>(_data, _location);

		// Get the vector size
		uint32_t vectorSize = PacketFileDataOperations::ReadFromData<uint32_t>(_data, _location);

		// For each vector entry
		for (int j = 0; j < vectorSize; j++)
		{
			PacketObjectManager::FileFragmentIdentifier fileFragmentIdentifier;

			// Get the full path size
			uint32_t fullPathSize = PacketFileDataOperations::ReadFromData<uint32_t>(_data, _location);

			// Read the full path
			std::string fullPath = PacketFileDataOperations::ReadFromData(_data, _location, fullPathSize);

			// Get the fragment name size
			uint32_t fragmentNameSize = PacketFileDataOperations::ReadFromData<uint32_t>(_data, _location);

			// Read the fragment name
			fileFragmentIdentifier.fragmentName = PacketFileDataOperations::ReadFromData(_data, _location, fragmentNameSize);

			// Read the fragment index
			fileFragmentIdentifier.fragmentIndex = PacketFileDataOperations::ReadFromData<uint32_t>(_data, _location);

			// Read the file identifier
			fileFragmentIdentifier.fileIdentifier = PacketFileDataOperations::ReadFromData<PacketFragment::FileIdentifier>(_data, _location);

			// Insert a new entry
			InsertEntry(key, fullPath, fileFragmentIdentifier);
		}
	}

	/* TODO
		- Deixar esse codigo menor
		- Usar estruturas de tamanho fixo (strings não são legais aqui)
		- Usar funções de serialize? (que implementa pra tanto json quanto binário?)
		- Usar uma forma de hashtable melhor ou mais inteligente (principalmente do que usar internamente vector)
	*/

	return true;
}

std::vector<unsigned char> Packet::PacketObjectHashTable::Serialize()
{
	int reservedMultiplier = 1;
	int reserveSize = 1000;
	unsigned char* _data = nullptr;
	uint32_t location = 0;

	// Save the total number of entries
	PacketFileDataOperations::SaveToData<uint32_t>(_data, location, (uint32_t)m_HashTable.size());

	// For each map entry
	for (std::map<uint32_t, std::vector<HashInternal>>::iterator it = m_HashTable.begin(); it != m_HashTable.end(); ++it)
	{
		// Get the vector
		std::vector<HashInternal>& internalVector = it->second;

		// Get the key
		uint32_t internalKey = it->first;

		// Save the key
		PacketFileDataOperations::SaveToData<uint32_t>(_data, location, internalKey);

		// Save the vector size
		PacketFileDataOperations::SaveToData<uint32_t>(_data, location, (uint32_t)internalVector.size());

		// For each vector entry
		for (int i = 0; i < internalVector.size(); i++)
		{
			// Get the entry
			HashInternal& internalEntry = internalVector[i];

			// Save the full path size
			PacketFileDataOperations::SaveToData<uint32_t>(_data, location, (uint32_t)internalEntry.fullPath.size());

			// Save the full path
			PacketFileDataOperations::SaveToData(_data, location, internalEntry.fullPath);

			// Save the fragment name size
			PacketFileDataOperations::SaveToData<uint32_t>(_data, location, (uint32_t)internalEntry.fragmentIdentifier.fragmentName.size());

			// Save the full path
			PacketFileDataOperations::SaveToData(_data, location, internalEntry.fragmentIdentifier.fragmentName);

			// Save the fragment name size
			PacketFileDataOperations::SaveToData<uint32_t>(_data, location, internalEntry.fragmentIdentifier.fragmentIndex);

			// Save the fragment name size
			PacketFileDataOperations::SaveToData<PacketFragment::FileIdentifier>(_data, location, internalEntry.fragmentIdentifier.fileIdentifier);
		}
	}
}

uint32_t Packet::PacketObjectHashTable::InsertEntry(std::string _path, PacketObjectManager::FileFragmentIdentifier& _internalIdentifier)
{
	// Hash the path
	uint32_t hash = HashFilePathNonStatic(_path.c_str());

	return InsertEntry(hash, _path, _internalIdentifier);
}

uint32_t Packet::PacketObjectHashTable::InsertEntry(uint32_t _key, std::string _path, PacketObjectManager::FileFragmentIdentifier& _internalIdentifier)
{
	// Create the new hash internal object
	HashInternal hashInternal;
	hashInternal.fullPath = _path;
	hashInternal.fragmentIdentifier = _internalIdentifier;

	// Find the entry
	auto it = m_HashTable.find(_key);
	if (it == m_HashTable.end())
	{
		// Create the new vector with the internal identifier
		std::vector<HashInternal> hashInternalVector;

		// Insert the new hash internal object
		hashInternalVector.push_back(hashInternal);

		// Insert it
		m_HashTable.insert(std::pair<uint32_t, std::vector<HashInternal>>(_key, hashInternalVector));

		return _key;
	}

	// Add the new hash internal object
	it->second.push_back(hashInternal);

	return _key;
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