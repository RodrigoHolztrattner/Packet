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
		return true;
	}

	void OnDataChanged(PacketResourceData& _data) override
	{
		return;
	}

	bool OnSynchronization()  override
	{
		return true;
	}

	bool OnDelete(PacketResourceData&) override
	{
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

	}

	void OnDependenciesFulfilled() override
	{

	}
};

class TextureFactory : public PacketResourceFactory
{
public:

	std::unique_ptr<PacketResourceInstance> RequestInstance(Hash _hash, PacketResourceManager* _resourceManager) override
	{
		return std::unique_ptr<PacketResourceInstance>(new TextureInstance(_hash, _resourceManager, this));
	}

	void ReleaseInstance(std::unique_ptr<PacketResourceInstance>& _instance) override
	{
		_instance.reset();
	}

	std::unique_ptr<PacketResource> RequestObject() override
	{
		return std::unique_ptr<Texture>(new Texture());
	}

	void ReleaseObject(std::unique_ptr<PacketResource>& _object) override
	{
		_object.reset();
	}

	bool AllocateData(PacketResourceData& _resourceDataRef, uint64_t _total) override
	{
		// Allocate the data
		return _resourceDataRef.AllocateMemory(_total);
	}

	void DeallocateData(PacketResourceData& _data) override
	{
		// Deallocate the memory
		_data.DeallocateMemory();
	}
};

int main()
{
	PacketSystem packetSystem;
	packetSystem.Initialize("Data", OperationMode::Condensed);

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

	bool result = packetSystem.GetReferenceManager()->ValidateFileReferences("Data\\test.txt", PacketReferenceManager::ReferenceFixer::NameAndExtensionOrSizeAndExtension);

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

	packetSystem.RequestObject(textureInstancePtr, "Data\\test.txt", &textureFactory);
	textureInstancePtr.Reset();
	while (true)
	{
		packetSystem.Update();

		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		if (textureInstancePtr && textureInstancePtr->IsReady())
		{
			std::cout << "IsReady!" << std::endl;

			
		}
	}

    return 0;
}