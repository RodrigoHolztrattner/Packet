#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <string>
#include <vector>
#include <fstream>

#include "..\Packet\Packet.h"
#include "MyResource.h"
#include "MyInstance.h"
#include "MyFactory.h"
#include "HelperMethods.h"

#define ResourceDirectory    std::string("Data")
#define MaximumTimeoutWaitMS long long(5000)

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

SCENARIO("Instances can request resources if their file exist", "[instance]")
{
    GIVEN("A packet system initialized on edit mode and registered with a MyFactory type resource factory")
    {
        Packet::System packetSystem;
        bool initializationResult = packetSystem.Initialize(Packet::OperationMode::Edit, ResourceDirectory);
        REQUIRE(initializationResult == true);

        packetSystem.RegisterResourceFactory<MyFactory, MyResource>();

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

SCENARIO("Instances can request runtime resources", "[instance]")
{
    GIVEN("A packet system initialized on edit mode and registered with a MyFactory type resource factory")
    {
        Packet::System packetSystem;
        bool initializationResult = packetSystem.Initialize(Packet::OperationMode::Edit, ResourceDirectory);
        REQUIRE(initializationResult == true);

        packetSystem.RegisterResourceFactory<MyFactory, MyResource>();

        WHEN("An instance request a runtime resource")
        {
            Packet::ResourceInstancePtr<MyInstance> resourceInstance;

            packetSystem.RequestRuntimeResource<MyResource>(resourceInstance);

            THEN("The instance must change its status to ready after some time")
            {
                REQUIRE(packetSystem.WaitForInstance(resourceInstance.Get(), MaximumTimeoutWaitMS) == true);
            }
        }
    }
}

/*
    - Instance pode ter um metodo que retorna um holder do seu recurso interno (incrementando a ref count?) que tenha 
    acesso tambem a propria instance, ao fazer isso a instance ficara com lock e apenas sera unlocked quando esse objeto
    for out of scope ou explicitamente chamarmos unlock nele.
    - O resource tambem deve respeitar o lock e unlock?
    - Esse objeto permite que funcoes do resource sejam chamadas usando o operador ->.
    - Ao finalizar as edicoes, o resource deve ser colocado para edicao no manager!?

    - Nao esquecer do external construct object!
*/