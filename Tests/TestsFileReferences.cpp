#include <catch2/catch.hpp>
#include <string>
#include <vector>
#include <fstream>

#include "../Packet/Packet.h"
#include "MyResource.h"
#include "MyFactory.h"
#include "HelperMethods.h"
#include "HelperDefines.h"

SCENARIO("Internal files depend and reference other internal files", "[references]")
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

        // Setup the names for all files that are going to be used on this test
        auto import_file_path = "Sounds/imported_file.olo";
        auto first_created_file_path = "Images/first_created_file.olo";
        auto second_created_file_path = "Images/second_created_file.olo";
        auto third_created_file_path = "Shaders/third_created_file.olo";

        // Import the initial file
        bool import_result = file_importer.ImportExternalFile(ExternalFilePath, import_file_path);
        REQUIRE(import_result == true);

        // Setup the initial data for the files that will be created
        Packet::FileType       file_type = "custom";
        std::vector<uint8_t>   icon_data = std::vector<uint8_t>();
        std::vector<uint8_t>   properties_data = std::vector<uint8_t>();
        std::vector<uint8_t>   original_data = std::vector<uint8_t>(100);
        std::vector<uint8_t>   intermediate_data = std::vector<uint8_t>(20);
        std::vector<uint8_t>   final_data = std::vector<uint8_t>(50);

        AND_GIVEN("Some files that were created with internal dependencies and links between themselves")
        {
            // First file
            {
                Packet::Path           target_path = first_created_file_path;
                std::vector<uint8_t>   file_icon_data = icon_data;
                std::vector<uint8_t>   file_properties_data = properties_data;
                std::vector<uint8_t>   file_original_data = original_data;
                std::vector<uint8_t>   file_intermediate_data = intermediate_data;
                std::vector<uint8_t>   file_final_data = final_data;
                std::set<Packet::Path> file_dependencies = std::set<Packet::Path>({ import_file_path });

                bool write_result = packetSystem.GetFileManager().WriteFile(
                    target_path,
                    file_type,
                    std::move(file_icon_data),
                    std::move(file_properties_data),
                    std::move(file_original_data),
                    std::move(file_intermediate_data),
                    std::move(file_final_data),
                    std::move(file_dependencies));
                REQUIRE(write_result == true);
            }

            // Second file
            {
                Packet::Path           target_path = second_created_file_path;
                std::vector<uint8_t>   file_icon_data = icon_data;
                std::vector<uint8_t>   file_properties_data = properties_data;
                std::vector<uint8_t>   file_original_data = original_data;
                std::vector<uint8_t>   file_intermediate_data = intermediate_data;
                std::vector<uint8_t>   file_final_data = final_data;
                std::set<Packet::Path> file_dependencies = std::set<Packet::Path>({ import_file_path, first_created_file_path });

                bool write_result = packetSystem.GetFileManager().WriteFile(
                    target_path,
                    file_type,
                    std::move(file_icon_data),
                    std::move(file_properties_data),
                    std::move(file_original_data),
                    std::move(file_intermediate_data),
                    std::move(file_final_data),
                    std::move(file_dependencies));
                REQUIRE(write_result == true);
            }

            // Third file
            {
                Packet::Path           target_path = third_created_file_path;
                std::vector<uint8_t>   file_icon_data = icon_data;
                std::vector<uint8_t>   file_properties_data = properties_data;
                std::vector<uint8_t>   file_original_data = original_data;
                std::vector<uint8_t>   file_intermediate_data = intermediate_data;
                std::vector<uint8_t>   file_final_data = final_data;
                std::set<Packet::Path> file_dependencies = std::set<Packet::Path>({ second_created_file_path });

                bool write_result = packetSystem.GetFileManager().WriteFile(
                    target_path,
                    file_type,
                    std::move(file_icon_data),
                    std::move(file_properties_data),
                    std::move(file_original_data),
                    std::move(file_intermediate_data),
                    std::move(file_final_data),
                    std::move(file_dependencies));
                REQUIRE(write_result == true);
            }

            WHEN("Those files are loaded")
            {
                // Load the files
                auto imported_file = file_loader.LoadFile(Packet::Hash(import_file_path));
                auto first_created_file = file_loader.LoadFile(Packet::Hash(first_created_file_path));
                auto second_created_file = file_loader.LoadFile(Packet::Hash(second_created_file_path));
                auto third_created_file = file_loader.LoadFile(Packet::Hash(third_created_file_path));
                REQUIRE(imported_file);
                REQUIRE(first_created_file);
                REQUIRE(second_created_file);
                REQUIRE(third_created_file);

                THEN("Their dependencies and links must match the ones specified on their creation")
                {
                    // Imported file
                    REQUIRE(imported_file->GetFileReferences().GetFileLinks().size() == 2);
                    REQUIRE(imported_file->GetFileReferences().GetFileDependencies().size() == 0);
                    REQUIRE(imported_file->GetFileReferences().GetFileLinks().find(first_created_file_path) != imported_file->GetFileReferences().GetFileLinks().end());
                    REQUIRE(imported_file->GetFileReferences().GetFileLinks().find(second_created_file_path) != imported_file->GetFileReferences().GetFileLinks().end());

                    // First created file
                    REQUIRE(first_created_file->GetFileReferences().GetFileLinks().size() == 1);
                    REQUIRE(first_created_file->GetFileReferences().GetFileDependencies().size() == 1);
                    REQUIRE(first_created_file->GetFileReferences().GetFileDependencies().find(import_file_path) != first_created_file->GetFileReferences().GetFileDependencies().end());
                    REQUIRE(first_created_file->GetFileReferences().GetFileLinks().find(second_created_file_path) != first_created_file->GetFileReferences().GetFileLinks().end());

                    // Second created file
                    REQUIRE(second_created_file->GetFileReferences().GetFileLinks().size() == 1);
                    REQUIRE(second_created_file->GetFileReferences().GetFileDependencies().size() == 2);
                    REQUIRE(second_created_file->GetFileReferences().GetFileDependencies().find(import_file_path) != second_created_file->GetFileReferences().GetFileDependencies().end());
                    REQUIRE(second_created_file->GetFileReferences().GetFileDependencies().find(first_created_file_path) != second_created_file->GetFileReferences().GetFileDependencies().end());
                    REQUIRE(second_created_file->GetFileReferences().GetFileLinks().find(third_created_file_path) != second_created_file->GetFileReferences().GetFileLinks().end());

                    // Third created file
                    REQUIRE(third_created_file->GetFileReferences().GetFileLinks().size() == 0);
                    REQUIRE(third_created_file->GetFileReferences().GetFileDependencies().size() == 1);
                    REQUIRE(third_created_file->GetFileReferences().GetFileDependencies().find(second_created_file_path) != third_created_file->GetFileReferences().GetFileDependencies().end());
                }
            }





            /*
            WHEN("A file is created/wrote")
            {
                THEN("The file must be located on the new path")
                {
                    REQUIRE(std::filesystem::exists(ResourceDirectory + "/" + target_path.path().string()) == true);
                }
            }
            */
        }
    }
}