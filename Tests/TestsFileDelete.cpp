#include <catch2/catch.hpp>
#include <string>
#include <vector>
#include <fstream>

#include "../Packet/Packet.h"
#include "MyResource.h"
#include "MyFactory.h"
#include "HelperMethods.h"
#include "HelperDefines.h"

SCENARIO("Internal files can be deleted", "[delete]")
{
    // Setup the playground
    SetupResourcePlayground();

    GIVEN("A packet system initialized on edit mode and an already existing packet file")
    {
        Packet::System packetSystem;
        bool packet_initialize_result = packetSystem.Initialize(Packet::OperationMode::Plain, ResourceDirectory);
        REQUIRE(packet_initialize_result == true);

        // Request the file importer, indexer and loader
        auto& file_importer = packetSystem.GetFileManager().GetFileImporter();
        auto& file_indexer = packetSystem.GetFileManager().GetFileIndexer();
        auto& file_loader = packetSystem.GetFileManager().GetFileLoader();

        // Setup the path we will import the file
        auto file_path = "Sounds/imported_file.pckfile";

        // Import the file
        bool import_result = file_importer.ImportExternalFile(ExternalFilePath, file_path);
        REQUIRE(import_result == true);

        WHEN("The packet file is deleted")
        {
            // Delete the file
            bool delete_result = packetSystem.GetFileManager().DeleteFile(file_path);

            AND_THEN("The delete result must be true")
            {
                CHECK(delete_result == true);
            }
        }

        WHEN("An invalid (non-existent) packet file is deleted")
        {
            // Delete the file
            bool delete_result = packetSystem.GetFileManager().DeleteFile("Sounds/invalid_file.pckfile");

            AND_THEN("The delete result must be false")
            {
                CHECK(delete_result == false);
            }
        }
    }
}