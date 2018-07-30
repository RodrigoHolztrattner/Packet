////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceLoader.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResourceLoader.h"
#include "PacketResourceFactory.h"
#include <cassert>
#include <chrono>

// Using namespace Peasant
PacketUsingDevelopmentNamespace(Packet)

PacketResourceLoader::PacketResourceLoader(PacketFileLoader* _fileLoaderPtr)
{
	// Save the file loader ptr
	m_FileLoaderPtr = _fileLoaderPtr;

	// Create the auxiliar thread
	m_AuxiliarThread = std::thread(&PacketResourceLoader::LoadObjectAuxiliar, this);
}

PacketResourceLoader::~PacketResourceLoader()
{
}

bool PacketResourceLoader::LoadObject(PacketResource* _object, Hash _hash, bool _isPermanent)
{
	// Set the data
	LoadData loadData = { _object, _hash, _isPermanent };

	return m_Queue.enqueue(loadData);
}

void PacketResourceLoader::Update()
{
	// For each object inside our synchronization queue
	PacketResource* object = nullptr;
	while (m_SynchronizationQueue.try_dequeue(object))
	{
		// Call the BeginSynchronization() method
		object->BeginSynchronization();
	}
}

void PacketResourceLoader::LoadObjectAuxiliar()
{
	// Do forever
	while (true)
	{
		// Try to get an object from the queue
		LoadData loadData;
		if (!m_Queue.try_dequeue(loadData))
		{
			// Sleep because there is no object to dequeue
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			continue;
		}

		// Check if the resource exist
		assert(m_FileLoaderPtr->FileExist(loadData.hash));

		// Get the resource size
		auto resourceSize = m_FileLoaderPtr->GetFileSize(loadData.hash);

		// Get a reference to the object data vector directly
		auto& dataVector = loadData.object->GetDataRef();

		// Using the object factory, allocate the necessary data
		bool result = loadData.object->GetFactoryPtr()->AllocateData(dataVector, resourceSize);
		assert(result);

		// Read the file data
		result = m_FileLoaderPtr->GetFileData(dataVector.GetData(), resourceSize, loadData.hash);
		assert(result);

		// Call the BeginLoad() method for this object
		result = loadData.object->BeginLoad(loadData.isPermanent);
		assert(result);
		
		// Insert the object into the synchronization queue
		m_SynchronizationQueue.enqueue(loadData.object);
	}
}