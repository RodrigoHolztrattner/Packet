////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceLoader.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "..\PacketConfig.h"
#include "PacketResource.h"
#include "PacketResourceStorage.h"

#include <readerwriterqueue/readerwriterqueue.h>
#include <thread>

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

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketResourceLoader
////////////////////////////////////////////////////////////////////////////////
class PacketResourceLoader
{
public:

	// The load data
	struct LoadData
	{
		// The object, the hash and if it is permanent
		PacketResource* object;
		Hash hash;
		bool isPermanent;
	};

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketResourceLoader(PacketFileLoader* _fileLoaderPtr);
	~PacketResourceLoader();

//////////////////
// MAIN METHODS //
public: //////////

	// Load a new object
	bool LoadObject(PacketResource* _object, Hash _hash, bool _isPermanent);

	// The update method
	void Update();

private:

	// The auxiliar load method
	void LoadObjectAuxiliar();

///////////////
// VARIABLES //
private: //////

	// The auxiliar thread
	std::thread m_AuxiliarThread;

	// The object queue
	moodycamel::ReaderWriterQueue<LoadData> m_Queue;

	// The object synchronization queue (callable only if the object was loaded)
	moodycamel::ReaderWriterQueue<PacketResource*> m_SynchronizationQueue;

	// The packet file loader ptr
	PacketFileLoader* m_FileLoaderPtr;
};

// Packet
PacketDevelopmentNamespaceEnd(Packet)
