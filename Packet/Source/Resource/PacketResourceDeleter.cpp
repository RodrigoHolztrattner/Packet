////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceDeleter.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResourceDeleter.h"
#include <cassert>
#include <chrono>

// Using namespace Peasant
PacketUsingDevelopmentNamespace(Packet)

PacketResourceDeleter::PacketResourceDeleter()
{
	// Create the auxiliar thread
	m_AuxiliarThread = std::thread(&PacketResourceDeleter::DeleteObjectAuxiliar, this);
}

PacketResourceDeleter::~PacketResourceDeleter()
{
}

bool PacketResourceDeleter::DeleteObject(std::unique_ptr<PacketResource>& _object, PacketResourceFactory* _factoryPtr, bool _deleteSync)
{
	// Create the delete request
	DeleteRequest deleteRequest = { std::move(_object), _factoryPtr, _deleteSync };

	return m_Queue.enqueue(std::move(deleteRequest));
}

void PacketResourceDeleter::Update()
{
	// For each object inside our factory deletion queue...
	FactoryDeleteRequest factoryDeleteRequest;
	while (m_FactoryDeletionQueue.try_dequeue(factoryDeleteRequest))
	{
		assert(!factoryDeleteRequest.object->IsReady());
		assert(!factoryDeleteRequest.object->IsReferenced());

		// Call the release method for the factory
		factoryDeleteRequest.factory->ReleaseObject(factoryDeleteRequest.object);
	}
}

void PacketResourceDeleter::DeleteObjectAuxiliar()
{
	// Do forever
	while (true)
	{
		// Try to get an object from the queue
		DeleteRequest deleteRequest;
		if (!m_Queue.try_dequeue(deleteRequest))
		{
			// Sleep because there is no object to dequeue
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			continue;
		}

		// Call the OnDelete() method for this object (to release the data)
		bool result = deleteRequest.object->BeginDelete();

		// Release the object data using its factory (this is safe because we are using the resource allocator)
		deleteRequest.factory->DeallocateData(deleteRequest.object->GetDataRef());

		assert(result == true);

		// Check if the factory should delete this object synchronous
		if (!deleteRequest.deleteSync)
		{
			assert(!deleteRequest.object->IsReady());
			assert(!deleteRequest.object->IsReferenced());

			// Call the release method for the factory assynchronous
			deleteRequest.factory->ReleaseObject(deleteRequest.object);
		}
		else
		{
			// Create a new factory deletion request
			FactoryDeleteRequest factoryDeleteRequest = { std::move(deleteRequest.object), deleteRequest.factory };

			// Insert into the factory deletion request
			m_FactoryDeletionQueue.enqueue(std::move(factoryDeleteRequest));
		}
	}
}