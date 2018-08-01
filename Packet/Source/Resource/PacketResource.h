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
class PacketSystem;

/*
	=> Novos métodos:

		ok - OnDataChange() (vai propagar um novo OnSynchronization() também)
		esse método deve invalidar (usando talvez um index?) possíveis usos do recurso de forma que uma instancia
		precise se atualizar (comparando um id interno?). Problema: Isso acontece ok no frame update mas e se o 
		recurso estiver em uso na render thread?
		O recurso pode se registrar para não receber esse método caso aconteça alguma edição, podendo escolher em 
		não receber caso a edição seja pelo programa mesmo ou externamente (diretamente no arquivo) ou ambos.
		Idem sobre o chamamendo da função OnSynchronization(), podendo ou não realizar o chamamento da mesma.

		ok - UpdateResourcePhysicalData() vai efetivamente alterar o arquivo físico do recurso, fazendo com que o 
		mesmo seja reescrito no seu local de existência, método falha caso o recurso não tenha sido salvo e tenha 
		sido criado "at runtime".

		- RegisterUpdateCondition() registra uma forma de realizar um UpdateResourcePhysicalData() caso algum dos 
		fatores especificados (flags) aconteçam, como por exemplo quando o recurso for sair de memória...

	=> Sobre alterações de recurso:

		- São apenas permitidas no modo editor do packet, cada recurso precisa saber se alterar, o que será 
		disponibilizado são funções de editar os dados do recurso e funções de notificação para as instancias 
		do mesmo, em um model por exemplo ele precisa ter funções de "add mesh" e "add texture", após realizar 
		as alterações desejadas podemos dar commit nelas, salvando os dados em disco e podendo ou não notificar 
		todas as instancias desse model por exemplo.

		
		- Criar resource diretamente? Essa é a melhor forma? Permitir construtores diferentes?
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

	enum ResourceUpdateConditionBits
	{
		ResourceUpdateOnRelease = 1 << 0

	};
	typedef uint32_t ResourceUpdateConditionFlags;

	// TODO: 
	void RegisterUpdateCondition(ResourceUpdateConditionFlags _updateFlags);

	// Update this resource physical data, overwritting it. This method will only works if the packet system is operating on 
	// edit mode, by default calling this method will result in a future deletion of this resource object and in a future 
	// creation of a new resource object with the updated data, if the user needs to update the data at runtime multiple times
	// is recomended to batch multiple "data updates" and call this method once in a while (only call this method when there is 
	// a real need to actually save the data)
	bool UpdateResourcePhysicalData(uint8_t* _data, uint32_t _dataSize);
	bool UpdateResourcePhysicalData(PacketResourceData& _data);

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

public:

	// Make a instance reference this object / remove reference 
	void MakeInstanceReference(PacketResourceInstance* _instance);
	void RemoveInstanceReference(PacketResourceInstance* _instance);

////////////////////
public: // STATUS //
////////////////////

	// Return if this object is ready to be used
	bool IsReady();

	// Return if this object is pending replacement
	bool IsPendingReplacement();

	// Return if this object is referenced
	bool IsReferenced();
	bool IsDirectlyReferenced();
	bool IsIndirectlyReferenced();

	// Return if this object is persistent (if it won't be released when it's reference count reaches 0)
	bool IsPersistent();

/////////////////////////
protected: // INTERNAL //
/////////////////////////

	// Begin load, deletion and synchronize methods
	bool BeginLoad(bool _isPersistent);
	bool BeginDelete();
	bool BeginSynchronization();

	// Set the hash
	void SetHash(Hash _hash);

	// Set the factory reference
	void SetFactoryReference(PacketResourceFactory* _factoryReference);

	// Set that this resource is pending replacement
	void SetPedingReplacement();

	// Make all instances that depends on this resource to point to another resource, decrementing the total 
	// number of references to zero, this method must be called when inside the update phase on the resource 
	// manager so no race conditions will happen. This method will only do something when on debug builds
	void RedirectInstancesToResource(PacketResource* _newResource);

	// This method will check if all instances that depends on this resource are totally constructed and ready
	// to be used, this method only works on debug builds and it's not intended to be used on release builds, 
	// also this method must be called when inside the update method on the PacketResourceManager class
	bool AreInstancesReadyToBeUsed();

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

	// The total number of direct and indirect references
	std::atomic<uint32_t> m_TotalDirectReferences;
	std::atomic<uint32_t> m_TotalIndirectReferences;

	// The resource data and hash
	PacketResourceData m_Data;
	Hash m_Hash;

	// The resource update condition flags
	ResourceUpdateConditionFlags m_UpdateConditionFlags;

	// A pointer to the resource factory and the packet system
	PacketResourceFactory* m_FactoryPtr;
	PacketSystem* m_PacketSystemPtr;

#ifndef NDEBUG

	// This is a vector with all instances that uses this object + a mutex to prevent multiple writes 
	// at the same time, those variables exist only on debug builds and they offer functionality to 
	// the runtime detection of resource file changes, providing a way to update all instances that 
	// points to a given resource object
	std::vector<PacketResourceInstance*> m_InstancesThatUsesThisResource;
	std::mutex m_InstanceVectorMutex;

#endif
};

// Packet
PacketDevelopmentNamespaceEnd(Packet)