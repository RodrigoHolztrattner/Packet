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

Packet::ResourceReferencePtr<MyResource> DummyMethod(Packet::ResourceReferencePtr<MyResource> _reference)
{
    return _reference;
}

SCENARIO("Resource references can be requested from instances", "[reference]")
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

                packetSystem.WaitForInstance(resourceInstance.Get());

                AND_WHEN("A resource reference is requested from that instance")
                {
                    Packet::ResourceReferencePtr<MyResource> resourceReference = resourceInstance->GetResourceReference<MyResource>();

                    THEN("The reference is valid")
                    {
                        REQUIRE(resourceReference.IsValid() == true);
                    }

                    AND_WHEN("The initial instance is reseted")
                    {
                        resourceInstance.Reset();

                        THEN("The number of resources inside the packet system will be kept at 1 since there is an active reference")
                        {
                            REQUIRE(MustNotChangeToFalseUntilTimeout([&]()
                            {
                                return packetSystem.GetAproximatedResourceAmount() == 1;
                            }, 1000));
                        }

                        AND_WHEN("The resource is moved to another variable")
                        {
                            Packet::ResourceReferencePtr<MyResource> otherResourceReference = std::move(resourceReference);

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
                            Packet::ResourceReferencePtr<MyResource> returnedResourceReference = DummyMethod(std::move(resourceReference));

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
    }
}