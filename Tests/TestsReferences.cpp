#include <catch2/catch.hpp>
#include <string>
#include <vector>
#include <fstream>

#include "..\Packet\Packet.h"
#include "MyResource.h"
#include "MyFactory.h"
#include "HelperMethods.h"
#include "HelperDefines.h"

Packet::ResourceReference<MyResource> DummyMethod(Packet::ResourceReference<MyResource> _reference)
{
    return _reference;
}

SCENARIO("Resource references can be be used to access resources", "[reference]")
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

            WHEN("An resource is requested")
            {
                Packet::ResourceReference<MyResource> resourceReference;

                packetSystem.RequestResource<MyResource>(
                    resourceReference,
                    Packet::Hash(resourcePath));

                packetSystem.WaitForResource(resourceReference);

                AND_WHEN("A new resource reference is created from the old one")
                {
                    Packet::ResourceReference<MyResource> newResourceReference = resourceReference;

                    THEN("The reference is valid")
                    {
                        REQUIRE(newResourceReference.IsValid() == true);
                    }

                    AND_WHEN("The initial reference is reseted")
                    {
                        resourceReference.Reset();

                        THEN("The number of resources inside the packet system will be kept at 1 since there is an active reference")
                        {
                            REQUIRE(MustNotChangeToFalseUntilTimeout([&]()
                            {
                                return packetSystem.GetAproximatedResourceAmount() == 1;
                            }, 1000));
                        }
                    }
                }

                AND_WHEN("The resource is moved to another variable")
                {
                    Packet::ResourceReference<MyResource> otherResourceReference = std::move(resourceReference);

                    THEN("The old reference must not be valid")
                    {
                        REQUIRE(resourceReference.IsValid() == false);
                    }

                    AND_THEN("The other reference must be valid")
                    {
                        REQUIRE(otherResourceReference.IsValid() == true);
                    }

                    AND_THEN("The number of resources inside the packet system will be kept at 1 since there is an active reference")
                    {
                        REQUIRE(MustNotChangeToFalseUntilTimeout([&]()
                        {
                            return packetSystem.GetAproximatedResourceAmount() == 1;
                        }, 1000));
                    }
                }

                AND_WHEN("The resource is passed to another function and captured back")
                {
                    Packet::ResourceReference<MyResource> returnedResourceReference = DummyMethod(std::move(resourceReference));

                    THEN("The old reference must not be valid")
                    {
                        REQUIRE(resourceReference.IsValid() == false);
                    }

                    AND_THEN("The returned reference must be valid")
                    {
                        REQUIRE(returnedResourceReference.IsValid() == true);
                    }

                    AND_THEN("The number of resources inside the packet system will be kept at 1 since there is an active reference")
                    {
                        REQUIRE(MustNotChangeToFalseUntilTimeout([&]()
                        {
                            return packetSystem.GetAproximatedResourceAmount() == 1;
                        }, 1000));
                    }
                }

                AND_WHEN("The resource is passed to another function and not captured back")
                {
                    DummyMethod(std::move(resourceReference));

                    AND_THEN("The number of resources on the packet system must be equal to zero after some time")
                    {
                        REQUIRE(MustChangeToTrueAfterTimeout([&]()
                        {
                            return packetSystem.GetAproximatedResourceAmount() == 0;
                        }, 5000));
                    }
                }
            }
        }
    }
}