#include <catch2/catch.hpp>
#include <string>
#include <vector>
#include <fstream>

#include "..\Packet\Packet.h"
#include "MyResource.h"
#include "MyFactory.h"
#include "HelperMethods.h"
#include "HelperDefines.h"

SCENARIO("External files can be imported into the packet system", "[import]")
{
    // Setup the playground
    SetupResourcePlayground();

    GIVEN("A packet system initialized on edit mode")
    {
        Packet::System packetSystem;
        packetSystem.Initialize(Packet::OperationMode::Plain, ResourceDirectory);

        // Request the file importer, indexer and loader
        auto& file_importer = packetSystem.GetFileManager().GetFileImporter();
        auto& file_indexer = packetSystem.GetFileManager().GetFileIndexer();
        auto& file_loader = packetSystem.GetFileManager().GetFileLoader();

        // Get the external file size
        size_t external_file_size = std::filesystem::file_size(ExternalFilePath);

        WHEN("An external file is imported")
        {
            bool import_result = file_importer.ImportExternalFile(ExternalFilePath, "Sounds/imported_file.olo");

            THEN("The import must be successful")
            {
                REQUIRE(import_result == true);
            }

            AND_THEN("The file must be indexed")
            {
                REQUIRE(file_indexer.IsFileIndexed(Packet::Hash("Sounds/imported_file.olo")) == true);
            }

            AND_WHEN("This new file is loaded as a packet file")
            {
                auto new_file = file_loader.LoadFile(Packet::Hash("Sounds/imported_file.olo"));

                THEN("The loaded file must be valid")
                {
                    REQUIRE(new_file != nullptr);
                }

                AND_THEN("Its header must contain the correct informations")
                {
                    REQUIRE(new_file->GetFileHeader().GetVersion() == Packet::Version);
                    REQUIRE(new_file->GetFileHeader().GetFileSize() > 0);
                    REQUIRE(new_file->GetFileHeader().GetFileType().string() == "default");
                    REQUIRE(new_file->GetFileHeader().GetPath().string() == "Sounds/imported_file.olo");
                    REQUIRE(new_file->GetFileHeader().GetDataSize(Packet::FilePart::FinalData) == external_file_size);
                }
            }
        }
    }
}