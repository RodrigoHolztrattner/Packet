////////////////////////////////////////////////////////////////////////////////
// Filename: PacketResource.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "..\PacketConfig.h"

#include <memory>
#include <cstdint>
#include <vector>
#include <atomic>
#include <mutex>

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
class PacketResourceManager;
class PacketResourceLoader;
class PacketResourceDeleter;
class PacketResourceFactory;
class PacketResourceWatcher;
class PacketReferenceManager;

template <typename ResourceClass>
class PacketResourceReferencePtr;

/*
	=> Novos m�todos:

		ok - OnDataChange() (vai propagar um novo OnSynchronization() tamb�m)
		esse m�todo deve invalidar (usando talvez um index?) poss�veis usos do recurso de forma que uma instancia
		precise se atualizar (comparando um id interno?). Problema: Isso acontece ok no frame update mas e se o 
		recurso estiver em uso na render thread?
		O recurso pode se registrar para n�o receber esse m�todo caso aconte�a alguma edi��o, podendo escolher em 
		n�o receber caso a edi��o seja pelo programa mesmo ou externamente (diretamente no arquivo) ou ambos.
		Idem sobre o chamamendo da fun��o OnSynchronization(), podendo ou n�o realizar o chamamento da mesma.

		ok - UpdateResourcePhysicalData() vai efetivamente alterar o arquivo f�sico do recurso, fazendo com que o 
		mesmo seja reescrito no seu local de exist�ncia, m�todo falha caso o recurso n�o tenha sido salvo e tenha 
		sido criado "at runtime".

		- RegisterUpdateCondition() registra uma forma de realizar um UpdateResourcePhysicalData() caso algum dos 
		fatores especificados (flags) aconte�am, como por exemplo quando o recurso for sair de mem�ria...

	=> Sobre altera��es de recurso:

		- S�o apenas permitidas no modo editor do packet, cada recurso precisa saber se alterar, o que ser� 
		disponibilizado s�o fun��es de editar os dados do recurso e fun��es de notifica��o para as instancias 
		do mesmo, em um model por exemplo ele precisa ter fun��es de "add mesh" e "add texture", ap�s realizar 
		as altera��es desejadas podemos dar commit nelas, salvando os dados em disco e podendo ou n�o notificar 
		todas as instancias desse model por exemplo.

		
		- Criar resource diretamente? Essa � a melhor forma? Permitir construtores diferentes?
*/

// The structure that will be used to store the resource data, it works like a vector of uint8_t and is allocation 
// and deallocation must be managed by a factory class
struct PacketResourceData
{
	// Our constructors
	PacketResourceData();
	PacketResourceData(uint8_t* _data, uint64_t _size);
	PacketResourceData(uint64_t _size);

	// Copy constructor
	PacketResourceData(PacketResourceData&);

	// The pointer deletion must be done manually
	~PacketResourceData();

	// Our move assignment operator
	PacketResourceData& operator=(PacketResourceData&& _other);

	// Our move copy operator (same as above)
	PacketResourceData(PacketResourceData&& _other);

	// Return the data
	uint8_t* GetData();

	// Return the size
	uint64_t GetSize();

	// Allocates memory for this object
	virtual bool AllocateMemory(uint64_t _total);

	// Deallocate this object memory
	virtual void DeallocateMemory();

protected:

	uint8_t* m_Data;
	uint64_t m_Size;
};

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketResource
////////////////////////////////////////////////////////////////////////////////
class PacketResource
{
public:

	// Friend classes
	friend PacketResourceManager;
	friend PacketResourceLoader;
	friend PacketResourceDeleter;
	friend PacketResourceWatcher;
	friend PacketResourceInstance;

	template <typename ResourceClass>
	friend class PacketResourceReferencePtr;

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketResource();
	virtual ~PacketResource();

/////////////////////
// VIRTUAL METHODS //
protected: //////////

	// The OnLoad() method (asynchronous method)
	virtual bool OnLoad(PacketResourceData& _data) = 0;

	// The OnSynchronization() method (synchronous method when calling the update() method)
	virtual bool OnSynchronization() = 0;

	// The OnDelete() method (asynchronous method)
	virtual bool OnDelete(PacketResourceData&) = 0;

//////////////////
// MAIN METHODS //
public: //////////

	// Return the resource hash
	Hash GetHash();

	// Return the data size
	uint32_t GetDataSize();

	// Return the total number of instances that directly or indirectly references this resource
	uint32_t GetTotalNumberReferences();
	uint32_t GetTotalNumberDirectReferences();
	uint32_t GetTotalNumberIndirectReferences();

	// Return the object factory without cast
	PacketResourceFactory* GetFactoryPtr();

	// Return the object factory casting to the given template typeclass
	template<typename FactoryClass>
	FactoryClass* GetFactoryPtr()
	{
		return reinterpret_cast<FactoryClass*>(m_Factory);
	}

//////////////////////////////////
public: // PHYSICAL DATA UPDATE //
//////////////////////////////////

	// This method will set this resource to ignore physical changes on its data, if the resource file happens to be 
	// modified or any call to UpdateResourcePhysicalData() method won't be resulting in the destruction of this 
	// resource and in a creation of a new one, this method will only works when on edit mode for the packet system 
	void IgnoreResourcePhysicalDataChanges();

	// Update this resource physical data, overwritting it. This method will only works if the packet system is operating on 
	// edit mode, by default calling this method will result in a future deletion of this resource object and in a creation
	// of a new resource object with the updated data, if the user needs to update the data at runtime multiple times is 
	// recomended to batch multiple "data updates" and call this method once in a while (only call this method when there is 
	// a real need to actually save the data)
	bool UpdateResourcePhysicalData(uint8_t* _data, uint64_t _dataSize);
	bool UpdateResourcePhysicalData(PacketResourceData& _data);

////////////////////////////////////////
public: // PHYSICAL RESOURCE REFERECE //
////////////////////////////////////////

	// Register a physical reference from this resource to another one path, doing this will allows the reference system to 
	// detect filename changes and modifications, allowing it to update this resource to point to the new resources, this 
	// method will return false if the current operation mode is different from the edit one or if this isn't a debug build.
	// The user must provide the location where the target resource hash data is located INSIDE this resource data, the 
	// location must indicates where this hash can be found when reading the resource file (doing a seek + this position).
	bool RegisterPhysicalResourceReference(Hash _targetResourceHash, uint64_t _hashDataLocation);

	// This method will clear all physical resource references for this current resource, the same rules above also apply to
	// this method (return false if the operation mode isn't the edit one or this isn't a debug build)
	bool ClearAllPhysicalResourceReferences();

	// This method will verify all resource physical references this resource have, checking if they exist, it's possible to 
	// pass a ReferenceFixer as argument that determine a behaviour to find any missing reference and fix it. For more info 
	// take a look at the documentation, use this method with caution.
	// The same rules above also apply to this method (return false if the operation mode isn't the edit one or this isn't 
	// a debug build)
	bool VerifyPhysicalResourceReferences(ReferenceFixer _fixer = ReferenceFixer::None, bool _allOrNothing = true);

////////////////////
public: // STATUS //
////////////////////

	// Return if this object is ready to be used
	bool IsReady();

	// Return if this object is pending replacement
	bool IsPendingReplacement();

	// Return if this object is pending deletion
	bool IsPendingDeletion();

	// Return if this object is referenced
	bool IsReferenced();
	bool IsDirectlyReferenced();
	bool IsIndirectlyReferenced();

	// Return if this object is persistent (if it won't be released when it's reference count reaches 0)
	bool IsPersistent();

/////////////////////////////////////
protected: // INSTANCE REFERENCING //
/////////////////////////////////////

	// Make a instance reference this object / remove reference 
	void MakeInstanceReference(PacketResourceInstance* _instance);
	void RemoveInstanceReference(PacketResourceInstance* _instance);

	// Make/remove a temporary reference
	void MakeTemporaryReference();
	void RemoveTemporaryReference();

/////////////////////////
protected: // INTERNAL //
/////////////////////////

	// Begin load, deletion and synchronize methods
	bool BeginLoad(bool _isPersistent);
	bool BeginDelete();
	bool BeginSynchronization();

	// Set the hash
	void SetHash(Hash _hash);

	// Set the helper object pointers and the operation mode
	void SetHelperObjects(PacketResourceFactory* _factoryReference, PacketReferenceManager* _referenceManager, PacketFileLoader* _fileLoader, OperationMode _operationMode);

	// Set that this resource is pending replacement
	void SetPedingReplacement();

	// Set that this resource is pending deletion
	void SetPendingDeletion();

	// Make all instances that depends on this resource to point to another resource, decrementing the total 
	// number of references to zero, this method must be called when inside the update phase on the resource 
	// manager so no race conditions will happen. This method will only do something when on debug builds
	void RedirectInstancesToResource(PacketResource* _newResource);

	// This method will check if all instances that depends on this resource are totally constructed and ready
	// to be used, this method only works on debug builds and it's not intended to be used on release builds, 
	// also this method must be called when inside the update method on the PacketResourceManager class
	bool AreInstancesReadyToBeUsed();

	// Return if this resource ignore physical data changes
	bool IgnorePhysicalDataChanges();

	// Return the data reference
	PacketResourceData& GetDataRef();

///////////////
// VARIABLES //
private: //////

	// If this object was loaded, if the data is valid and if this object was synchronized
	bool m_DataValid;
	bool m_WasSynchronized;
	bool m_IsPersistent;
	bool m_IsPendingReplacement;
	bool m_IgnorePhysicalDataChanges;
	bool m_IsPendingDeletion;

	// The total number of direct and indirect references
	std::atomic<uint32_t> m_TotalDirectReferences;
	std::atomic<uint32_t> m_TotalIndirectReferences;

	// The resource data and hash
	PacketResourceData m_Data;
	Hash m_Hash;

	// A pointer to the resource factory, the reference manager and the file loader
	PacketResourceFactory* m_FactoryPtr;
	PacketReferenceManager* m_ReferenceManagerPtr;
	PacketFileLoader* m_FileLoaderPtr;

	// The current operation mode for the packet system
	OperationMode m_CurrentOperationMode;

#ifndef NDEBUG

	// This is a vector with all instances that uses this object + a mutex to prevent multiple writes 
	// at the same time, those variables exist only on debug builds and they offer functionality to 
	// the runtime detection of resource file changes, providing a way to update all instances that 
	// points to a given resource object
	std::vector<PacketResourceInstance*> m_InstancesThatUsesThisResource;
	std::mutex m_InstanceVectorMutex;

#endif
};

// The temporary resource reference type
template <typename ResourceClass>
class PacketResourceReferencePtr
{
	// Friend classes
	friend PacketResource;
	friend PacketResourceInstance;

protected:

	// Constructor used by the resource class
	PacketResourceReferencePtr(ResourceClass* _resource) : m_ResourceObject(_resource) {}

public:

	// Default constructor
	PacketResourceReferencePtr() : m_ResourceObject(nullptr) {}

	// Copy constructor disabled
	PacketResourceReferencePtr(const PacketResourceReferencePtr&) = delete;

	// Move operator
	PacketResourceReferencePtr& operator=(PacketResourceReferencePtr&& _other)
	{
		// Set the pointer
		m_ResourceObject = _other.m_ResourceObject;
		_other.m_ResourceObject = nullptr;
	}

	// Our move copy operator (same as above)
	PacketResourceReferencePtr(PacketResourceReferencePtr&& _other)
	{
		// Set the pointer
		m_ResourceObject = _other.m_ResourceObject;
		_other.m_ResourceObject = nullptr;
	}

	// Operator to use this as a pointer to the resource object
	ResourceClass* operator->() const
	{
		return m_ResourceObject;
	}

	// Destructor
	~PacketResourceReferencePtr()
	{
		// If the resource object is valid
		if (m_ResourceObject != nullptr)
		{
			m_ResourceObject->RemoveTemporaryReference();
			m_ResourceObject = nullptr;
		}
	}

private:

	// The resource object
	ResourceClass* m_ResourceObject = nullptr;
};

// Packet
PacketDevelopmentNamespaceEnd(Packet)