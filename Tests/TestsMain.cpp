#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <string>
#include <vector>
#include <fstream>

#include "..\Packet\Packet.h"
#include "MyResource.h"
#include "MyFactory.h"
#include "HelperMethods.h"
#include "HelperDefines.h"

SCENARIO("Packet system can be initialized", "[system]")
{
    GIVEN("A non initialized packet system")
    {
        Packet::System packetSystem;

        WHEN("It's initialized in edit mode")
        {
            bool initializationResult = packetSystem.Initialize(Packet::OperationMode::Plain, ResourceDirectory);

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