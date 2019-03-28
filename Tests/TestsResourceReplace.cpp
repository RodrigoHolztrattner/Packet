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

SCENARIO("Resources can be replaced if their file content changes", "[replace]")
{
    GIVEN("A packet system initialized on edit mode and registered with a MyFactory type resource factory")
    {
        Packet::System packetSystem;
        packetSystem.Initialize(Packet::OperationMode::Edit, ResourceDirectory);

        packetSystem.RegisterResourceFactory<MyFactory, MyResource>();

        AND_GIVEN("A previously created resource file")
        {
            std::string resourcePath = ResourceDirectory + "/dummy.txt";
            CreateResourceFile(resourcePath);

            WHEN("An instance request that resource")
            {
                Packet::ResourceInstancePtr<MyInstance> resourceInstance;

                packetSystem.RequestResource<MyResource>(
                    resourceInstance,
                    Packet::Hash(resourcePath));
            
                packetSystem.WaitForInstance(resourceInstance.Get(), MaximumTimeoutWaitMS);

                THEN("The number of calls for the instance method must reflect the current usage")
                {
                    REQUIRE(resourceInstance->GetOnConstructTotalCalls() == 1);
                    REQUIRE(resourceInstance->GetOnDependenciesFulfilledTotalCalls() == 1);
                    REQUIRE(resourceInstance->GetOnDeleteTotalCalls() == 0);
                }

                AND_WHEN("The resource file changes")
                {
                    UpdateResourceFile(resourcePath);

                    THEN("The number of calls for the instance method must reflect the current usage after some time")
                    {
                        REQUIRE(MustChangeToTrueUntilTimeout([&]()
                        {
                            return resourceInstance->GetOnConstructTotalCalls() == 2;
                        }, MaximumTimeoutWaitMS));

                        REQUIRE(MustChangeToTrueUntilTimeout([&]()
                        {
                            return resourceInstance->GetOnDependenciesFulfilledTotalCalls() == 2;
                        }, MaximumTimeoutWaitMS));

                        REQUIRE(MustChangeToTrueUntilTimeout([&]()
                        {
                            return resourceInstance->GetOnDeleteTotalCalls() == 1;
                        }, MaximumTimeoutWaitMS));
                    } 
                }
            }
        }
    }
}