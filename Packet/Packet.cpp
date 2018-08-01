// Packet.cpp : Defines the entry point for the console application.
//

#include "Packet.h"

#include <iostream>

/*
	=> Arquivos na raiz do projeto que eu preciso ter (arquivos ocultos):

		- Lista com todos os arquivos e pastas que fazem parte do sistema, essa lista deve ser atualizada sempre que entrarmos no edit mode.
		- Lista com o catálogo 

	=> O arquivo com as referencias só deve ser checado quando o arquivo em si for "especionado" (sei la como se fala) usando um validador
*/

/*
	- Um arquivo pode comportar que seja editado ou não
*/

/*
	=> Resource
	=> Resource reference
	=> Resource temporal reference

*/

/*
	-> Carregar arquivos igual ao Peasant
	-> Poder criar um resource diretamente sem ter que carregá-lo
	-> Poder salvar um resource (pode precisar de flags especiais)
	-> Poder editar um resource (pode precisar de flags especiais)
*/

#include "PacketSystem.h"

PacketUsingDevelopmentNamespace(Packet)

#include <fstream>

#include <thread>

#include "Resource/PacketResourceInstance.h"

class Texture : public PacketResource
{
public:

	bool OnLoad(PacketResourceData& _data) override
	{
		std::cout << (int)this << " - Resource -> OnLoad() - data size: " << _data.GetSize() << std::endl;
		return true;
	}

	bool OnSynchronization()  override
	{
		std::cout << (int)this << " - Resource -> OnSynchronization()" << std::endl;
		return true;
	}

	bool OnDelete(PacketResourceData&) override
	{
		std::cout << (int)this << " - Resource -> OnDelete()" << std::endl;
		return true;
	}
};

class TextureInstance : public PacketResourceInstance
{
public:

	TextureInstance(Hash _hash, PacketResourceManager* _resourceManager, PacketResourceFactory* _factoryPtr) : PacketResourceInstance(_hash, _resourceManager, _factoryPtr)
	{

	}

	void OnConstruct() override
	{
		std::cout << (int)this << " - Instance -> OnConstruct()" << std::endl;
	}

	void OnDependenciesFulfilled() override
	{
		std::cout << (int)this << " - Instance -> OnDependenciesFulfilled()" << std::endl;
	}

	void OnReset() override
	{
		std::cout << (int)this << " - Instance -> OnReset()" << std::endl;
	}
};

class TextureFactory : public PacketResourceFactory
{
public:

	std::unique_ptr<PacketResourceInstance> RequestInstance(Hash _hash, PacketResourceManager* _resourceManager) override
	{
		auto object = std::unique_ptr<PacketResourceInstance>(new TextureInstance(_hash, _resourceManager, this));
		std::cout << "Factory -> RequestInstance() - ptr: " << (int)object.get() << std::endl;
		return object;
	}

	void ReleaseInstance(std::unique_ptr<PacketResourceInstance>& _instance) override
	{
		std::cout << "Factory -> ReleaseInstance() - ptr: " << (int)_instance.get() << std::endl;
		_instance.reset();
	}

	std::unique_ptr<PacketResource> RequestObject() override
	{
		auto object = std::unique_ptr<Texture>(new Texture());
		std::cout << "Factory -> RequestObject() - ptr: " << (int)object.get() << std::endl;
		return object;
	}

	void ReleaseObject(std::unique_ptr<PacketResource>& _object) override
	{
		std::cout << "Factory -> ReleaseObject() - ptr: " << (int)_object.get() << std::endl;
		_object.reset();
	}

	bool AllocateData(PacketResourceData& _resourceDataRef, uint64_t _total) override
	{
		std::cout << "Factory -> AllocateData() - total: " << _total << std::endl;
		// Allocate the data
		return _resourceDataRef.AllocateMemory(_total);
	}

	void DeallocateData(PacketResourceData& _data) override
	{
		std::cout << "Factory -> DeallocateData() - total: " << _data.GetSize() << std::endl;
		// Deallocate the memory
		_data.DeallocateMemory();
	}
};

int main()
{
	PacketSystem packetSystem;
	packetSystem.Initialize("Data", OperationMode::Edit);

	//

	/*
	// Open the file and check if we are ok to proceed
	std::ofstream file("Data\\test.txt", std::ios::binary);

	Path path;

	path = "Data\\gems prices.png";
	file.write((char*)&path, sizeof(Path));

	path = "Data\\Textures\\linhas terrain unreal.png";
	file.write((char*)&path, sizeof(Path));

	file.close();
	*/

	//

	bool result = packetSystem.GetReferenceManager()->ValidateFileReferences("Data\\test.txt", ReferenceFixer::NameAndExtensionOrSizeAndExtension);

	// packetSystem.GetReferenceManager()->RegisterFileReference("Data\\test.txt", "Data\\gems prices.png", 0);
	// packetSystem.GetReferenceManager()->RegisterFileReference("Data\\test.txt", "Data\\Textures\\linhas terrain unreal.png", sizeof(Path));

	//

	packetSystem.ConstructPacket();

	//
	//
	//

	//

	PacketResourceInstancePtr<TextureInstance> textureInstancePtr;
	TextureFactory textureFactory;

	packetSystem.RequestObject(textureInstancePtr, "Data\\texto.txt", &textureFactory);
	while (true)
	{
		packetSystem.Update();

		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		if (textureInstancePtr && !textureInstancePtr->IsReady())
		{
			std::cout << "Not ready!" << std::endl;

			// textureInstancePtr.Reset();

		}
	}

    return 0;
}