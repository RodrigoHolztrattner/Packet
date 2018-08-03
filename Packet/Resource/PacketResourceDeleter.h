////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResourceDeleter.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "..\PacketConfig.h"
#include "PacketResource.h"
#include "..\ThirdParty\readerwriterqueue\readerwriterqueue.h"

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

// Classes we know
class PacketResourceFactory;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketResourceDeleter
////////////////////////////////////////////////////////////////////////////////
class PacketResourceDeleter
{
public:

	// The delete request type
	struct DeleteRequest
	{
		// The object and factory
		std::unique_ptr<PacketResource> object;
		PacketResourceFactory* factory;

		// If this object should be deleted synchronous
		bool deleteSync;
	};

	// The factory delete request
	struct FactoryDeleteRequest
	{
		// The object and factory
		std::unique_ptr<PacketResource> object;
		PacketResourceFactory* factory;
	};

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketResourceDeleter();
	~PacketResourceDeleter();

//////////////////
// MAIN METHODS //
public: //////////

	// Load a new object
	bool DeleteObject(std::unique_ptr<PacketResource>& _object, PacketResourceFactory* _factoryPtr, bool _deleteSync);

	// The update method
	void Update();

private:

	// The auxiliar delete method
	void DeleteObjectAuxiliar();

///////////////
// VARIABLES //
private: //////

	// The auxiliar thread
	std::thread m_AuxiliarThread;

	// The object queue
	moodycamel::ReaderWriterQueue<DeleteRequest> m_Queue;

	// The factory deletion queue (used to delete the object itself by the factory in sync mode)
	moodycamel::ReaderWriterQueue<FactoryDeleteRequest> m_FactoryDeletionQueue;
};

// Packet
PacketDevelopmentNamespaceEnd(Packet)
