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
public:

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

#define ResourceDirectory    std::string("Data")
#define MaximumTimeoutWaitMS long long(5000)

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

SCENARIO("Packet system can be initialized", "[system]")
{
    GIVEN("A non initialized packet system")
    {
        Packet::System packetSystem;

        WHEN("It's initialized in edit mode") 
        {
            bool initializationResult = packetSystem.Initialize(Packet::OperationMode::Edit, ResourceDirectory);

            THEN("It must have been initialized successfully") 
            {
                REQUIRE(initializationResult == true);
            }
        }

        WHEN("It's initialized in condensed mode") 
        {
            bool initializationResult = packetSystem.Initialize(Packet::OperationMode::Condensed, ResourceDirectory);

            THEN("It must have been initialized successfully") 
            {
                REQUIRE(initializationResult == true);
            }
        }
    }
}

SCENARIO("Instances can request resources if they exist", "[instance]")
{
    GIVEN("A packet system initialized on edit mode and a registered with a MyFactory type resource factory")
    {
        Packet::System packetSystem;
        bool initializationResult = packetSystem.Initialize(Packet::OperationMode::Edit, ResourceDirectory);
        REQUIRE(initializationResult == true);

        packetSystem.RegisterResourceFactory<MyResource>(std::make_unique<MyFactory>());

        AND_GIVEN("A previously created resource file")
        {
            std::string resourcePath = ResourceDirectory + "/dummy.txt";
            bool resourceCreationResult = CreateResourceFile(resourcePath);
            REQUIRE(resourceCreationResult == true);

            WHEN("An instance request that resource")
            {
                Packet::ResourceInstancePtr<MyInstance> resourceInstance;

                bool requestResult = packetSystem.RequestResource<MyResource>(
                    resourceInstance,
                    Packet::Hash(resourcePath));

                THEN("The request must have returned true (the resource file exist)")
                {
                    REQUIRE(requestResult == true);
                }

                AND_THEN("The instance must change its status to ready after some time")
                {
                    REQUIRE(packetSystem.WaitForInstance(resourceInstance.Get(), MaximumTimeoutWaitMS) == true);
                }
            }
        }

        WHEN("An instance request a resource that doesn't exist")
        {
            Packet::ResourceInstancePtr<MyInstance> resourceInstance;

            bool requestResult = packetSystem.RequestResource<MyResource>(
                resourceInstance,
                Packet::Hash("bla.txt"));

            THEN("The request must have returned false (the resource file doesn't exist)")
            {
                REQUIRE(requestResult == false);
            }
        }
    }
}