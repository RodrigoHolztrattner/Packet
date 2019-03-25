#include <catch2/catch.hpp>
#include <string>
#include <vector>
#include <fstream>

#include "..\Packet\Packet.h"
#include "MyResource.h"
#include "MyInstance.h"
#include "MyFactory.h"
#include "HelperMethods.h"
#include "HelperDefines.h"

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

                    AND_WHEN("The instance is reseted")
                    {
                        resourceInstance.Reset();

                        THEN("The number of resources on the packet system must be equal to zero after some time")
                        {
                            REQUIRE(MustChangeToTrueUntilTimeout([&]()
                            {
                                return packetSystem.GetAproximatedResourceAmount() == 0;
                            }, 5000));
                        }
                    }
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

                AND_WHEN("The instance is reseted")
                {
                    resourceInstance.Reset();

                    THEN("The number of resources on the packet system must be equal to zero after some time")
                    {
                        REQUIRE(MustChangeToTrueUntilTimeout([&]()
                        {
                            return packetSystem.GetAproximatedResourceAmount() == 0;
                        }, 5000));
                    }
                }
            }
        }
    }
}

SCENARIO("Instances can request permanent resources if their file exist", "[instance]")
{
    GIVEN("A packet system initialized on edit (doesn't matter) mode and registered with a MyFactory type resource factory")
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

            WHEN("An instance request a permanent resource")
            {
                Packet::ResourceInstancePtr<MyInstance> resourceInstance;

                bool requestResult = packetSystem.RequestPermanentResource<MyResource>(
                    resourceInstance,
                    Packet::Hash(resourcePath));

                THEN("The request must have returned true (the resource file exist)")
                {
                    REQUIRE(requestResult == true);
                }

                AND_THEN("The instance must change its status to ready after some time")
                {
                    REQUIRE(packetSystem.WaitForInstance(resourceInstance.Get(), MaximumTimeoutWaitMS) == true);

                    AND_WHEN("The instance is reseted")
                    {
                        resourceInstance.Reset();

                        THEN("The number of resources on the packet system must not change and be 1 since the resource is permanent")
                        {
                            REQUIRE(MustChangeToTrueAfterTimeout([&]()
                            {
                                return packetSystem.GetAproximatedResourceAmount() == 1;
                            }, 500));
                        }
                    }
                }

                AND_THEN("The instance is reseted")
                {
                    resourceInstance.Reset();

                    THEN("The number of resources on the packet system must not change and be 1 since the resource is permanent")
                    {
                        REQUIRE(MustChangeToTrueAfterTimeout([&]()
                        {
                            return packetSystem.GetAproximatedResourceAmount() == 1;
                        }, 500));
                    }
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

SCENARIO("Multiple resource requests and releases can be made from multiple threads at the same time", "[instance]")
{
    GIVEN("A default packet system initialized on any mode (doesn't matter what we choose) and a resource file")
    {
        // Packet system
        Packet::System packetSystem;
        bool initializationResult = packetSystem.Initialize(Packet::OperationMode::Edit, ResourceDirectory);
        packetSystem.RegisterResourceFactory<MyFactory, MyResource>();

        // Resource file
        std::string resourcePath = ResourceDirectory + "/dummy.txt";
        bool resourceCreationResult = CreateResourceFile(resourcePath);

        WHEN("Multiple threads request resources at the same time")
        {
            const int TotalRequestPerThread = 100;
            std::array<Packet::ResourceInstancePtr<MyInstance>, TotalRequestPerThread> instancesThread_a;
            std::array<Packet::ResourceInstancePtr<MyInstance>, TotalRequestPerThread> instancesThread_b;
            std::array<Packet::ResourceInstancePtr<MyInstance>, TotalRequestPerThread> instancesThread_c;
            std::array<Packet::ResourceInstancePtr<MyInstance>, TotalRequestPerThread> instancesThread_d;

            std::thread a = std::thread([&]()
            {
                for (int i = 0; i < TotalRequestPerThread; i++)
                {
                    bool requestResult = packetSystem.RequestResource<MyResource>(
                        instancesThread_a[i],
                        Packet::Hash(resourcePath));
                }
            });

            std::thread b = std::thread([&]()
            {
                for (int i = 0; i < TotalRequestPerThread; i++)
                {
                    bool requestResult = packetSystem.RequestResource<MyResource>(
                        instancesThread_b[i],
                        Packet::Hash(resourcePath));
                }
            });

            std::thread c = std::thread([&]()
            {
                for (int i = 0; i < TotalRequestPerThread; i++)
                {
                    bool requestResult = packetSystem.RequestResource<MyResource>(
                        instancesThread_c[i],
                        Packet::Hash(resourcePath));
                }
            });

            std::thread d = std::thread([&]()
            {
                for (int i = 0; i < TotalRequestPerThread; i++)
                {
                    bool requestResult = packetSystem.RequestResource<MyResource>(
                        instancesThread_d[i],
                        Packet::Hash(resourcePath));
                }
            });

            THEN("Eventually all instances must be ready for use")
            {
                for (int i = 0; i < TotalRequestPerThread; i++)
                {
                    REQUIRE(packetSystem.WaitForInstance(instancesThread_a[i].Get(), MaximumTimeoutWaitMS) == true);
                    REQUIRE(packetSystem.WaitForInstance(instancesThread_b[i].Get(), MaximumTimeoutWaitMS) == true);
                    REQUIRE(packetSystem.WaitForInstance(instancesThread_c[i].Get(), MaximumTimeoutWaitMS) == true);
                    REQUIRE(packetSystem.WaitForInstance(instancesThread_d[i].Get(), MaximumTimeoutWaitMS) == true);
                }
            }

            AND_THEN("The number of resources on the packet system must be equal to 1 after some time")
            {
                REQUIRE(MustChangeToTrueUntilTimeout([&]()
                {
                    return packetSystem.GetAproximatedResourceAmount() == 1;
                }, 5000));
            }

            AND_WHEN("After all instances were created, if we release them all but one")
            {
                // Wait for their creation
                a.join();
                b.join();
                c.join();
                d.join();

                for (int i = 0; i < TotalRequestPerThread; i++)
                {
                    instancesThread_a[i].Reset();
                    instancesThread_b[i].Reset();
                    instancesThread_c[i].Reset();
                    if (i != 99)
                    {
                        instancesThread_d[i].Reset();
                    }
                }

                THEN("The number of resources on the packet system must not change and be 1 since one instance still uses the resource")
                {
                    REQUIRE(MustChangeToTrueAfterTimeout([&]()
                    {
                        return packetSystem.GetAproximatedResourceAmount() == 1;
                    }, 1000));
                }

                AND_WHEN("We release the remaining instance")
                {
                    instancesThread_d[99].Reset();

                    THEN("The number of resources on the packet system must be equal to zero after some time")
                    {
                        REQUIRE(MustChangeToTrueUntilTimeout([&]()
                        {
                            return packetSystem.GetAproximatedResourceAmount() == 0;
                        }, 5000));
                    }
                }
            }

            if (a.joinable())
                a.join();
            if (b.joinable())
                b.join();
            if (c.joinable())
                c.join();
            if (d.joinable())
                d.join();
        }
    }
}