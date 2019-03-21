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

PacketResource* PacketResourceStorage::FindObject(Hash _hash, uint32_t _buildFlags) const
{
	// Check if an object with the given hash exist
	auto iterator = m_ObjectMap.find({ _hash, _buildFlags });
	if (iterator == m_ObjectMap.end())
	{
		return nullptr;
	}

	return iterator->second.get();
}

bool PacketResourceStorage::InsertObject(std::unique_ptr<PacketResource> _object, Hash _hash, uint32_t _buildFlags)
{
	// Check if an object with the given hash exist
	auto iterator = m_ObjectMap.find({ _hash, _buildFlags });
	if (iterator != m_ObjectMap.end())
	{
		return false;
	}

	// Insert the object
	m_ObjectMap.insert(std::make_pair(std::make_pair(_hash, _buildFlags), std::move(_object)));

	return true;
}

std::unique_ptr<PacketResource> PacketResourceStorage::ReplaceObject(std::unique_ptr<PacketResource>& _object, Hash _hash, uint32_t _buildFlags)
{
	// Check if an object with the given hash exist
	auto iterator = m_ObjectMap.find({ _hash, _buildFlags });
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
	return RemoveObject(_object->GetHash(), _object->GetBuildInfo().buildFlags);
}

bool PacketResourceStorage::RemoveObject(Hash _hash, uint32_t _buildFlags)
{
	// Check if an object with the given hash exist
	auto iterator = m_ObjectMap.find({ _hash, _buildFlags });
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
	auto iterator = m_ObjectMap.find({ _object->GetHash(), _object->GetBuildInfo().buildFlags });
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

const std::map<std::pair<HashPrimitive, uint32_t>, std::unique_ptr<PacketResource>>& PacketResourceStorage::GetObjectMapReference() const
{
	return m_ObjectMap;
}