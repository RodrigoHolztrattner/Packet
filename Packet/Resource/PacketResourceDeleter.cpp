////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceDeleter.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResourceDeleter.h"
#include "PacketResourceFactory.h"
#include "PacketResource.h"
#include <cassert>
#include <chrono>

// Using namespace Peasant
PacketUsingDevelopmentNamespace(Packet)

PacketResourceDeleter::PacketResourceDeleter()
{
}

PacketResourceDeleter::~PacketResourceDeleter()
{
}

bool PacketResourceDeleter::DeleteObject(std::unique_ptr<PacketResource> _object,
                                         PacketResourceFactory* _factoryPtr) const
{
	// Call the BeginDesynchronization() method for this resource
	_object->BeginDesynchronization();

    // Release the object data using its factory (this is safe because we are using the resource allocator)
    _factoryPtr->DeallocateData(_object->GetDataRef());

    // Call the release method for the factory asynchronous
    _factoryPtr->ReleaseObject(std::move(_object));

    return true;
}
