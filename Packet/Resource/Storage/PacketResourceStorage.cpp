////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceStorage.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResourceStorage.h"
#include "../PacketResource.h"

#include <cassert>

// Using namespace Peasant
PacketUsingDevelopmentNamespace(Packet)

PacketResourceStorage::PacketResourceStorage(OperationMode _operation_mode) :
    m_OperationMode(_operation_mode)
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

std::set<PacketResource*> PacketResourceStorage::GetAllObjectsWithHash(Hash _hash) const
{
    if (m_HashLinkedResourcesMap.find(_hash) == m_HashLinkedResourcesMap.end())
    {
        return {};
    }

    return m_HashLinkedResourcesMap.find(_hash)->second;
}

bool PacketResourceStorage::InsertObject(std::unique_ptr<PacketResource> _object, Hash _hash, uint32_t _buildFlags)
{
    auto InsertHashLinkedMapObject = [&](PacketResource* _object, HashPrimitive _hash_primitive)
    {
        auto iter = m_HashLinkedResourcesMap.find(_hash_primitive);
        if (iter == m_HashLinkedResourcesMap.end())
        {
            m_HashLinkedResourcesMap.insert({ _hash_primitive, {_object } });
        }
        else
        {
            iter->second.insert(_object);
        }
    };

	// Check if an object with the given hash exist
	auto iterator = m_ObjectMap.find({ _hash, _buildFlags });
	if (iterator != m_ObjectMap.end())
	{
		return false;
	}

    // If the current operation mode is the Plain one, add the resource into the hash linked resource map
    if (m_OperationMode == OperationMode::Plain)
    {
        InsertHashLinkedMapObject(_object.get(), _hash);
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

    // If the current operation mode is the Plain one, update the object stored on the hash linked map
    if (m_OperationMode == OperationMode::Plain)
    {
        assert(m_HashLinkedResourcesMap.find(_hash) != m_HashLinkedResourcesMap.end());

        // Find the set the old object is stored at
        auto hash_set = m_HashLinkedResourcesMap.find(_hash)->second;

        // Remove the old object and insert the new one
        hash_set.erase(iterator->second.get());
        hash_set.insert(_object.get());
    }

	// Take ownership from the object
	std::unique_ptr<PacketResource> oldResource = std::move(iterator->second);

	// Replace the old object
	iterator->second = std::move(_object);

	return oldResource;
}

std::unique_ptr<PacketResource> PacketResourceStorage::GetObjectOwnership(PacketResource* _object)
{
    auto RemoveHashLinkedMapObject = [&](PacketResource* _object)
    {
        assert(m_HashLinkedResourcesMap.find(_object->GetHash()) != m_HashLinkedResourcesMap.end());
        m_HashLinkedResourcesMap.erase(_object->GetHash());
    };

	// Check if an object with the given hash exist
	auto iterator = m_ObjectMap.find({ _object->GetHash(), _object->GetBuildInfo().buildFlags });
	if (iterator == m_ObjectMap.end())
	{
		return nullptr;
	}

    // If the current operation mode is the Plain one, remove the resource from the hash linked resource map
    if (m_OperationMode == OperationMode::Plain)
    {
        RemoveHashLinkedMapObject(_object);
    }

	// Get the unique ptr
	std::unique_ptr<PacketResource> objectUniquePtr = std::move(iterator->second);

	// Erase the current object on the iterator
	m_ObjectMap.erase(iterator);

	// Return the unique ptr
	return std::move(objectUniquePtr);
}

std::vector<std::unique_ptr<PacketResource>> PacketResourceStorage::GetPermanentResourcesOwnership()
{
    auto RemoveHashLinkedMapObject = [&](PacketResource * _object)
    {
        assert(m_HashLinkedResourcesMap.find(_object->GetHash()) != m_HashLinkedResourcesMap.end());
        m_HashLinkedResourcesMap.erase(_object->GetHash());
    };

    std::vector<std::unique_ptr<PacketResource>> returnVector;

    for (auto it = m_ObjectMap.begin(); it != m_ObjectMap.end() ; )
    {
        if (it->second->IsPermanent())
        {
            // If the current operation mode is the Plain one, remove the resource from the hash linked resource map
            if (m_OperationMode == OperationMode::Plain)
            {
                RemoveHashLinkedMapObject(it->second.get());
            }

            returnVector.push_back(std::move(it->second));

            m_ObjectMap.erase(it++);
        }
        else
        {
            ++it;
        }
    }

    return returnVector;
}

uint32_t PacketResourceStorage::GetAproximatedResourceAmount() const
{
    return static_cast<uint32_t>(m_ObjectMap.size());
}