#include <catch2/catch.hpp>
#include <string>
#include <vector>
#include <fstream>

#include "..\Packet\Packet.h"
#include "MyResource.h"
#include "MyFactory.h"
#include "HelperMethods.h"
#include "HelperDefines.h"

SCENARIO("Resources can be replaced if their file content changes", "[replace]")
{
    GIVEN("A packet system initialized on edit mode and registered with a MyFactory type resource factory")
    {
        std::string resourcePath = "dummy.txt";
        CreateResourceFile(ResourceDirectory + "/" + resourcePath);

        Packet::System packetSystem;
        packetSystem.Initialize(Packet::OperationMode::Plain, ResourceDirectory);

        // Request the resource manager and register our factory
        auto& resource_manager = packetSystem.GetResourceManager();
        resource_manager.RegisterResourceFactory<MyFactory, MyResource>();

        AND_GIVEN("A previously created resource file")
        {


            WHEN("An resource is requested")
            {
                Packet::ResourceReference<MyResource> resourceReference;

                resource_manager.RequestResource<MyResource>(
                    resourceReference,
                    Packet::Hash(resourcePath));

                resource_manager.WaitForResource(resourceReference, MaximumTimeoutWaitMS);

                MyResource* internalResourcePtr = resourceReference.Get();

                AND_WHEN("The resource file changes")
                {
                    UpdateResourceFile(ResourceDirectory + "/" + resourcePath);

                    THEN("The reference resource must be updated to reference the new resource some time after")
                    {
                        REQUIRE(MustChangeToTrueUntilTimeout([&]()
                        {
                            return resourceReference.Get() != internalResourcePtr;
                        }, 1000));
                    }
                }
            }
        }
    }
}