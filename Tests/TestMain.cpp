#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <string>
#include <vector>
#include <fstream>

#include "..\Packet\Packet.h"

class MyResource : public Packet::Resource
{
public:

    MyResource()
    {
    }

    ~MyResource()
    {
    }

    bool OnLoad(Packet::ResourceData& _data, uint32_t _buildFlags, uint32_t _flags) final
    {
        return true;
    }

    bool OnDelete(Packet::ResourceData& _data) final
    {
        return true;
    }

    void OnConstruct() final
    {

    };

    void OnExternalConstruct() final
    {

    };
};

class MyInstance : public Packet::ResourceInstance
{
    //////////////////
    // CONSTRUCTORS //
public: //////////

    MyInstance(Packet::Hash& _hash, 
               Packet::ResourceManager* _resourceManager,
               Packet::ResourceFactory* _factoryPtr) :
        Packet::ResourceInstance(_hash,
                                 _resourceManager, 
                                 _factoryPtr)
    {
    }

    ~MyInstance()
    {
    }

    void OnConstruct() final
    {
    }

    void OnDependenciesFulfilled() final
    {
    }

    void OnReset() final
    {
    }
};

class MyFactory : public Packet::ResourceFactory
{
public:

    MyFactory() : Packet::ResourceFactory()
    {
        // Set the initial data
        // ...
    }

    ~MyFactory()
    {
    }

    std::unique_ptr<Packet::ResourceInstance> RequestInstance(Packet::Hash _hash, Packet::ResourceManager* _resourceManager)
    {
        return std::unique_ptr<Packet::ResourceInstance>(new MyInstance(_hash, _resourceManager, this));
    }

    void ReleaseInstance(std::unique_ptr<Packet::ResourceInstance> _instance) final
    {
        _instance.reset();
    }

    std::unique_ptr<Packet::Resource> RequestObject() final
    {
        return std::unique_ptr<Packet::Resource>(new MyResource());
    }

    void ReleaseObject(std::unique_ptr<Packet::Resource> _object) final
    {
        _object.reset();
    }

    bool AllocateData(Packet::ResourceData& _data, uint64_t _total) final
    {
        _data.AllocateMemory(_total);

        return true;
    }

    void DeallocateData(Packet::ResourceData& _data) final
    {
        _data.DeallocateMemory();
    }
};

static std::string ResourceDirectory = "Data";

bool CreateResourceFile(std::string _filename = ResourceDirectory + "/dummy.txt", uint32_t _amountToWrite = 100)
{
    std::filesystem::create_directory(ResourceDirectory);

    std::ofstream myfile(_filename);
    if (myfile.is_open())
    {
        for (int i = 0; i < _amountToWrite; i++)
        {
            myfile << std::to_string(_amountToWrite);
        }

        myfile.close();

        return true;
    }

    return false;
}

TEST_CASE("vectors can be sized and resized", "[vector]") 
{
    Packet::System packetSystem;
    std::string resourcePath = ResourceDirectory + "/dummy.txt";

    {
        bool resourceCreationResult = CreateResourceFile(resourcePath);
        REQUIRE(resourceCreationResult == true);
    }

    {
        bool packetSystemInitializationResult = packetSystem.Initialize(Packet::OperationMode::Edit, ResourceDirectory);
        REQUIRE(packetSystemInitializationResult == true);

        packetSystem.RegisterResourceFactory<MyResource>(std::make_unique<MyFactory>());
    }
    
    SECTION("resizing bigger changes size and capacity") 
    {
        Packet::ResourceInstancePtr<MyInstance> resourceInstance;

        packetSystem.RequestResource<MyResource>(resourceInstance,
                                                 Packet::Hash(resourcePath));

        bool waitResult = packetSystem.WaitUntilReady(resourceInstance.Get(), 5000);

        REQUIRE(waitResult == true);
    }
}