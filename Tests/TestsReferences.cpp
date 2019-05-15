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
    return std::move(_reference);
}

SCENARIO("Resource references can be be used to access resources", "[reference]")
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

                AND_WHEN("We wait until the resource reference is valid")
                {
                    resource_manager.WaitForResource(resourceReference);

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
                                                                             return resource_manager.GetAproximatedResourceAmount() == 1;
                                                                         }, 1000));
                            }
                        }
                    }
                }

                AND_WHEN("The resource is moved to another variable")
                {
                    Packet::ResourceReference<MyResource> otherResourceReference = std::move(resourceReference);

                    AND_WHEN("We wait this new reference variable to be valid")
                    {
                        resource_manager.WaitForResource(otherResourceReference);

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
                                                                         return resource_manager.GetAproximatedResourceAmount() == 1;
                                                                     }, 1000));
                        }
                    }   
                }

                AND_WHEN("The resource is passed to another function and captured back and also waited on")
                {
                    Packet::ResourceReference<MyResource> returnedResourceReference = DummyMethod(std::move(resourceReference));

                    resource_manager.WaitForResource(returnedResourceReference);

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
                            return resource_manager.GetAproximatedResourceAmount() == 1;
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
                            return resource_manager.GetAproximatedResourceAmount() == 0;
                        }, 5000));
                    }
                }
            }
        }
    }
}