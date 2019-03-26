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

PacketResource* PacketResourceStorage::FindObject(Hash _hash, uint32_t _buildFlags, bool _isRuntimeResource) const
{
    // If this is a runtime resource request, return nullptr, this is necessary to avoid unnecessary checks
    // on the manager class
    if (_isRuntimeResource)
    {
        return nullptr;
    }

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
    // Verify if the resource is a runtime one
    if (_object->IsRuntime())
    {
        // Insert it into the runtime object map
        m_RuntimeObjectMap.insert({ _object.get(), std::move(_object) });

        return true;
    }

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

std::unique_ptr<PacketResource> PacketResourceStorage::GetObjectOwnership(PacketResource* _object)
{
    // Verify if the resource is a runtime one
    if (_object->IsRuntime())
    {
        // Check if an object with the given ptr exist
        auto iterator = m_RuntimeObjectMap.find(_object);
        if (iterator == m_RuntimeObjectMap.end())
        {
            return nullptr;
        }

        // Get the unique ptr
        std::unique_ptr<PacketResource> objectUniquePtr = std::move(iterator->second);

        // Erase the current object on the iterator
        m_RuntimeObjectMap.erase(iterator);

        // Return the unique ptr
        return std::move(objectUniquePtr);
    }

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

std::vector<std::unique_ptr<PacketResource>> PacketResourceStorage::GetPermanentResourcesOwnership()
{
    std::vector<std::unique_ptr<PacketResource>> returnVector;

    for (auto it = m_ObjectMap.begin(); it != m_ObjectMap.end() ; )
    {
        if (it->second->IsPermanent())
        {
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
    return static_cast<uint32_t>(m_ObjectMap.size() + m_RuntimeObjectMap.size());
}