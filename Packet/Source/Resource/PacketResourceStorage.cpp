////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceStorage.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResourceStorage.h"
#include "PacketResource.h"

#include <cassert>

// Using namespace Peasant
PacketUsingDevelopmentNamespace(Packet)

PacketResourceStorage::PacketResourceStorage()
{
	// Set the initial data
	// ...
}

PacketResourceStorage::~PacketResourceStorage()
{
}

PacketResource* PacketResourceStorage::FindObject(Hash _hash)
{
	// Check if an object with the given hash exist
	auto iterator = m_ObjectMap.find(_hash);
	if (iterator == m_ObjectMap.end())
	{
		return nullptr;
	}

	return iterator->second.get();
}

bool PacketResourceStorage::InsertObject(std::unique_ptr<PacketResource>& _object, Hash _hash)
{
	// Check if an object with the given hash exist
	auto iterator = m_ObjectMap.find(_hash);
	if (iterator != m_ObjectMap.end())
	{
		return false;
	}

	// Insert the object
	m_ObjectMap.insert(std::pair<Hash, std::unique_ptr<PacketResource>>(_hash, std::move(_object)));

	return true;
}

std::unique_ptr<PacketResource> PacketResourceStorage::ReplaceObject(std::unique_ptr<PacketResource>& _object, Hash _hash)
{
	// Check if an object with the given hash exist
	auto iterator = m_ObjectMap.find(_hash);
	if (iterator == m_ObjectMap.end())
	{
		// Nothing we can replace
		return nullptr;
	}

	// Take ownership from the object
	std::unique_ptr<PacketResource> oldResource = std::move(iterator->second);

	// Replace the old object
	iterator->second = std::move(_object);

	return oldResource;
}

bool PacketResourceStorage::RemoveObject(PacketResource* _object)
{
	return RemoveObject(_object->GetHash());
}

bool PacketResourceStorage::RemoveObject(Hash _hash)
{
	// Check if an object with the given hash exist
	auto iterator = m_ObjectMap.find(_hash);
	if (iterator == m_ObjectMap.end())
	{
		return false;
	}

	// Erase the current object on the iterator
	m_ObjectMap.erase(iterator);

	return true;
}

std::unique_ptr<PacketResource> PacketResourceStorage::GetObjectOwnership(PacketResource* _object)
{
	// Check if an object with the given hash exist
	auto iterator = m_ObjectMap.find(_object->GetHash());
	if (iterator == m_ObjectMap.end())
	{
		return nullptr;
	}

	// Get the unique ptr
	std::unique_ptr<PacketResource> objectUniquePtr = std::move(iterator->second);

	// Erase the current object on the iterator
	m_ObjectMap.erase(iterator);

	// Return the unique ptr
	return std::move(objectUniquePtr);
}

std::map<HashPrimitive, std::unique_ptr<PacketResource>>& PacketResourceStorage::GetObjectMapReference()
{
	return m_ObjectMap;
}