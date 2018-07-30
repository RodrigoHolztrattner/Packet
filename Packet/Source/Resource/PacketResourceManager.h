////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceManager.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketResource.h"
#include "PacketResourceStorage.h"
#include "PacketResourceFactory.h"
#include "PacketMultipleQueue.h"
#include "PacketResourceLoader.h"
#include "PacketResourceDeleter.h"

#include <cstdint>
#include <vector>

///////////////
// NAMESPACE //
///////////////

/////////////
// DEFINES //
/////////////

////////////
// GLOBAL //
////////////

///////////////
// NAMESPACE //
///////////////

// Packet
PacketDevelopmentNamespaceBegin(Packet)

//////////////
// TYPEDEFS //
//////////////

////////////////
// FORWARDING //
////////////////

// Classes we know
class PacketResourceInstance;
template <typename InstanceClass>
struct PacketResourceInstancePtr;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketResourceManager
////////////////////////////////////////////////////////////////////////////////
class PacketResourceManager
{
public:

	// Friend classes
	friend PacketResourceInstance;

	// The request type
	struct ObjectRequest
	{
		// The instance and hash variables
		PacketResourceInstance* instance;
		Hash hash;

		// The factory ptr
		PacketResourceFactory* factoryPtr;

		// If the object is permanent
		bool isPermanent;
	};

	// The release type
	struct ObjectRelease
	{
		// The instance
		std::unique_ptr<PacketResourceInstance> instance;

		// The factory ptr
		PacketResourceFactory* factoryPtr;

		// If this object should be deleted synchronous
		bool deleteSync;
	};

	// The temporary object release
	struct TemporaryObjectRelease
	{
		// The object
		PacketResource* object;

		// The factory ptr
		PacketResourceFactory* factoryPtr;

		// If this object should be deleted synchronous
		bool deleteSync;
	};

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketResourceManager(PacketResourceStorage* _storagePtr, PacketFileLoader* _fileLoaderPtr, uint32_t _workerThreads, ThreadIndexRetrieveMethod _threadIndexMethod);
	~PacketResourceManager();
	
//////////////////
// MAIN METHODS //
public: //////////

	// Request an object for the given instance and resource hash
	template <typename ResourceInstance, typename ResourceFactory>
	bool RequestObject(PacketResourceInstancePtr<ResourceInstance>& _instancePtr, Hash _hash, ResourceFactory* _factoryPtr, bool _isPersistent, bool _allowAsynchronousConstruct = true)
	{
		// Asserts
		assert(!m_InUpdatePhase || (m_InUpdatePhase && std::this_thread::get_id() == m_UpdateThreadID));

		// Create a new resource instance object
		std::unique_ptr<PacketResourceInstance> newInstance = _factoryPtr->RequestInstance(_hash, this);

		// Get a pointer to this new instance because we will move it into the instance ptr
		PacketResourceInstance* newInstancePtr = newInstance.get();

		// Link the new instance with the instance ptr, takes ownership from the unique_ptr
		_instancePtr.InstanceLink(newInstance);

		// Check if we already have an object with this hash, if the object was loaded and if we can construct this instance asynchronous
		PacketResource* object = m_ResourceStoragePtr->FindObject(_hash);
		if (object != nullptr && object->IsReady() && _allowAsynchronousConstruct)
		{
			// In case we are requesting a pesistent object, check if the already loaded object was created using the permanent mode
			assert(!_isPersistent || (_isPersistent && object->IsPersistent()) && "Trying to request a permanent object but it was already loaded and is not permanent!");

			// Make the instance reference it
			object->MakeInstanceReference(newInstancePtr);

			// Construct this instance
			newInstancePtr->BeginConstruction();
		}
		else
		{
			// Create the new request
			ObjectRequest request = { newInstancePtr, _hash, _factoryPtr, _isPersistent };

			// Push the new request
			m_ObjectRequests.Insert(request);
		}

		return true;
	}

	// The update method, process all requests
	void Update();

protected:

	// Release an object instance, this method must be called only by a packet resource instance ptr when it is deleted without
	// being moved to another variable using move semantics
	void ReleaseObject(std::unique_ptr<PacketResourceInstance>& _instance, PacketResourceFactory* _factoryPtr, bool _allowAsynchronousDeletion = false);
	void ReleaseObject(PacketResource* _object, PacketResourceFactory* _factoryPtr, bool _allowAsynchronousDeletion = false);

	// TODO: Maybe remove this?
	// Query the loaded objects map, returning a reference to it
	std::map<HashPrimitive, std::unique_ptr<PacketResource>>& QueryLoadedObjects();

///////////////
// VARIABLES //
private: //////

	// The mutex we will use to secure thread safety
	std::mutex m_Mutex;

	// The object loader and deleter
	PacketResourceLoader m_ObjectLoader;
	PacketResourceDeleter m_ObjectDeleter;

	// The object requests and the release queue
	MultipleQueue<ObjectRequest> m_ObjectRequests;
	MultipleQueue<ObjectRelease> m_InstanceReleases;
	MultipleQueue<TemporaryObjectRelease> m_TemporaryInstanceReleases;

	// The construct queue
	std::vector<PacketResourceInstance*> m_ConstructQueue;

	// The object storage ptr
	PacketResourceStorage* m_ResourceStoragePtr;
	
	// If we are inside the update phase and the updating thread id (used for asserts only)
	bool m_InUpdatePhase;
	std::thread::id m_UpdateThreadID;
};

// Packet
PacketDevelopmentNamespaceEnd(Packet)