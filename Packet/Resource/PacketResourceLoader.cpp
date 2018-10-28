////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceLoader.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketResourceLoader.h"
#include "PacketResourceFactory.h"
#include "PacketResource.h"
#include "..\PacketReferenceManager.h"

#include <cassert>
#include <chrono>

// Using namespace Peasant
PacketUsingDevelopmentNamespace(Packet)

PacketResourceLoader::PacketResourceLoader(PacketFileLoader* _fileLoaderPtr, PacketReferenceManager* _referenceManager, PacketLogger* _loggerPtr, OperationMode _operationMode)
{
	// Save the pointers and the operation mode
	m_FileLoaderPtr = _fileLoaderPtr;
	m_ReferenceManagerPtr = _referenceManager;
	m_LoggerPtr = _loggerPtr;
	m_OperationMode = _operationMode;

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

		// Check if the file exist
#ifndef NDEBUG

		// If we shouldn't create the resource if its file doesn't exist (also if we aren't on edit mode)
		if (m_OperationMode != OperationMode::Edit || !loadData.object->GetBuildInfo().createResourceIfInexistent)
		{
			// Check if the resource exist
			if (!m_FileLoaderPtr->FileExist(loadData.hash))
			{
				// Error validating this resource references, we will continue but keep in mind that this resource needs to update it references hashes
				m_LoggerPtr->LogError(std::string("Trying to load a resource but it doesn't exist, path: \"")
					.append(loadData.hash.GetPath().String())
					.append("\"")
					.c_str());

				assert(false);
			}
		}
#endif

		// Check the operation mode to know if we should validate this resource references
		if (m_OperationMode == OperationMode::Edit)
		{
			// Validate this resource file references (if they exist), also try to fix them if they are invalid
			bool validationResult = m_ReferenceManagerPtr->ValidateFileReferences(loadData.hash.GetPath().String(), ReferenceFixer::AtLeastNameAndExtension);
			if (!validationResult)
			{
				// Error validating this resource references, we will continue but keep in mind that this resource needs to update it references hashes
				m_LoggerPtr->LogWarning(std::string("Found an error when validating a resource references (\"")
					.append(loadData.hash.GetPath().String())
					.append("\", we will continue but keep in mind that this resource needs to update it references hashes")
					.c_str());
			}
		}

		// If we file doesn't exist and we should create the resource in this case (also we must be on edit mode)
		if (loadData.object->GetBuildInfo().createResourceIfInexistent && m_FileLoaderPtr->FileExist(loadData.hash) && m_OperationMode == OperationMode::Edit)
		{
			// Call the BeginCreation() method for this object
			bool result = loadData.object->BeginCreation(loadData.isPermanent);
			assert(result);

			// Check if we should call the BeginLoad() method
			if (loadData.object->GetBuildInfo().createdResourceShouldLoad)
			{
				// Call the BeginLoad() method for this object
				loadData.object->BeginLoad(loadData.isPermanent);
				assert(result);
			}
		}
		// Normally create the resource
		else
		{
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
		}
		
		// Insert the object into the synchronization queue
		m_SynchronizationQueue.enqueue(loadData.object);
	}
}