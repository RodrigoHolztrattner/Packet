#include <catch2/catch.hpp>
#include <string>
#include <vector>
#include <fstream>

#include "../Packet/Packet.h"
#include "MyResource.h"
#include "MyFactory.h"
#include "HelperMethods.h"
#include "HelperDefines.h"

SCENARIO("Internal files can be created", "[create]")
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
        auto import_file_dir = "Sounds/";

        // Import the file
        auto import_result = file_importer.ImportExternalFile(ExternalFilePath, import_file_dir);
        REQUIRE(import_result);
        auto import_file_path = import_result.value().string();

        AND_GIVEN("Some valid file data")
        {
            Packet::Path           target_path       = "Sounds/created_file.pckfile";
            std::vector<uint8_t>   icon_data         = std::vector<uint8_t>();
            std::vector<uint8_t>   properties_data   = std::vector<uint8_t>();
            std::vector<uint8_t>   original_data     = std::vector<uint8_t>(100);
            std::vector<uint8_t>   intermediate_data = std::vector<uint8_t>(20);
            std::vector<uint8_t>   final_data        = std::vector<uint8_t>(50);
            std::set<Packet::Path> file_dependencies = std::set<Packet::Path>({ import_file_path });

            WHEN("A file is created/wrote")
            {
                bool write_result = packetSystem.GetFileManager().WriteFile(
                    target_path,
                    std::move(icon_data),
                    std::move(properties_data),
                    std::move(original_data),
                    std::move(intermediate_data),
                    std::move(final_data),
                    std::move(file_dependencies));

                THEN("The file must have been created")
                {
                    REQUIRE(write_result == true);
                }

                THEN("The file must be located on the new path")
                {
                    CHECK(std::filesystem::exists(ResourceDirectory + "/" + target_path.path().string()) == true);
                }

                AND_THEN("The file path must be indexed")
                {
                    CHECK(file_indexer.IsFileIndexed(Packet::Hash(target_path)) == true);
                }

                AND_THEN("The file data must valid")
                {
                    // Load the file from the new path
                    auto created_file = file_loader.LoadFile(Packet::Hash(target_path));
                    REQUIRE(created_file);
                    
                    CHECK(created_file->GetFileHeader().GetVersion() == Packet::Version);
                    CHECK(created_file->GetFileHeader().GetFileSize() > 0);
                    CHECK(created_file->GetFileHeader().path() == target_path);
                    CHECK(created_file->GetIconData().size() == 0);
                    CHECK(created_file->GetPropertiesData().size() == 0);
                    CHECK(created_file->GetOriginalData().size() == 100);
                    CHECK(created_file->GetIntermediateData().size() == 20);
                    CHECK(created_file->GetFinalData().size() == 50);
                    CHECK(created_file->GetReferencesData().size() > 0);
                    CHECK(created_file->GetFileReferences().GetFileLinks().size() == 0);
                    CHECK(created_file->GetFileReferences().GetFileDependencies().size() == 1);
                    CHECK(created_file->GetFileReferences().GetFileDependencies().find(import_file_path) != created_file->GetFileReferences().GetFileDependencies().end());
                    CHECK(created_file->IsExternalFile() == false);
                }

                AND_THEN("The initial imported file must have a link with this recently created file since it depends on this initial imported file")
                {
                    // Load the initial imported
                    auto imported_file = file_loader.LoadFile(Packet::Hash(import_file_path));
                    REQUIRE(imported_file != nullptr);

                    CHECK(imported_file->GetFileReferences().GetFileLinks().size() == 1);
                    CHECK(imported_file->GetFileReferences().GetFileLinks().find(target_path) != imported_file->GetFileReferences().GetFileLinks().end());
                }
            }
        }
    }
}