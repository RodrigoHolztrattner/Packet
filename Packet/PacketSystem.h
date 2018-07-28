////////////////////////////////////////////////////////////////////////////////
// Filename: PacketSystem.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketReferenceManager.h"

#include <string>

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

// Packet data explorer
PacketDevelopmentNamespaceBegin(Packet)

////////////////
// FORWARDING //
////////////////

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketSystem
////////////////////////////////////////////////////////////////////////////////
class PacketSystem
{
public:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketSystem();
	~PacketSystem();

//////////////////
// MAIN METHODS //
public: //////////

	// Initialize this object
	bool Initialize(std::string _packetFolderPath, OperationMode _operationMode);

	// Pack all files
	bool ConstructPacket();

	// Return a ptr to the reference manager
	PacketReferenceManager* GetReferenceManager();

public:

	// Request an object for the given instance and resource hash
//	bool RequestObject(PeasantInstance& _instance, PeasantHash _hash, PeasantObjectFactory* _factoryPtr, bool _allowAsynchronousConstruct = false);

	// Request a permanent object for the given instance and resource hash, the object will not be deleted when it reaches 0
	// references, the deletion phase will only occur in conjunction with the storage deletion
//	bool RequestPersistentObject(PeasantInstance& _instance, PeasantHash _hash, PeasantObjectFactory* _factoryPtr, bool _allowAsynchronousConstruct = false);

	// Release an object instance
//	void ReleaseObject(PeasantInstance& _instance, PeasantObjectFactory* _factoryPtr, bool _allowAsynchronousDeletion = false);

	// Check if a given file exist
	bool FileExist(Hash _fileHash);

	// Query the loaded objects map, returning a reference to it
//	std::map<PeasantHash, PeasantObject*>& QueryLoadedObjects();

	// The update method, process all requests
//	void Update();

///////////////
// VARIABLES //
private: //////

	// The operation mode
	OperationMode m_OperationMode;

	// The packet folder path
	std::string m_PacketFolderPath;

private:

	// Our packet file loader
	std::unique_ptr<PacketFileLoader> m_FileLoader;

	// The reference manager
	PacketReferenceManager m_ReferenceManager;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
