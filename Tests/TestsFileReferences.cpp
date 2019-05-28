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

        // Setup the names for all paths that are going to be used on this test
        auto import_file_dir         = "Sounds/";
        auto first_created_file_path  = "Images/first_created_file.pckfile";
        auto second_created_file_path = "Images/second_created_file.pckfile";
        auto third_created_file_path  = "Shaders/third_created_file.pckfile";
        auto move_file_dir            = "Shaders";
        auto copied_file_dir          = "Images";
        auto renamed_file_name        = "renamed_file.pckfile";

        // Import the initial file
        auto import_result = file_importer.ImportExternalFile(ExternalFilePath, import_file_dir);
        REQUIRE(import_result);
        auto import_file_path = import_result.value().string();

        // Setup the initial data for the files that will be created
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
                    std::move(file_icon_data),
                    std::move(file_properties_data),
                    std::move(file_original_data),
                    std::move(file_intermediate_data),
                    std::move(file_final_data),
                    std::move(file_dependencies));
                REQUIRE(write_result == true);
            }

            WHEN("These files are loaded")
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
                    CHECK(imported_file->GetFileReferences().GetFileLinks().size() == 2);
                    CHECK(imported_file->GetFileReferences().GetFileDependencies().size() == 0);
                    CHECK(imported_file->GetFileReferences().GetFileLinks().find(first_created_file_path) != imported_file->GetFileReferences().GetFileLinks().end());
                    CHECK(imported_file->GetFileReferences().GetFileLinks().find(second_created_file_path) != imported_file->GetFileReferences().GetFileLinks().end());

                    // First created file
                    CHECK(first_created_file->GetFileReferences().GetFileLinks().size() == 1);
                    CHECK(first_created_file->GetFileReferences().GetFileDependencies().size() == 1);
                    CHECK(first_created_file->GetFileReferences().GetFileDependencies().find(import_file_path) != first_created_file->GetFileReferences().GetFileDependencies().end());
                    CHECK(first_created_file->GetFileReferences().GetFileLinks().find(second_created_file_path) != first_created_file->GetFileReferences().GetFileLinks().end());

                    // Second created file
                    CHECK(second_created_file->GetFileReferences().GetFileLinks().size() == 1);
                    CHECK(second_created_file->GetFileReferences().GetFileDependencies().size() == 2);
                    CHECK(second_created_file->GetFileReferences().GetFileDependencies().find(import_file_path) != second_created_file->GetFileReferences().GetFileDependencies().end());
                    CHECK(second_created_file->GetFileReferences().GetFileDependencies().find(first_created_file_path) != second_created_file->GetFileReferences().GetFileDependencies().end());
                    CHECK(second_created_file->GetFileReferences().GetFileLinks().find(third_created_file_path) != second_created_file->GetFileReferences().GetFileLinks().end());

                    // Third created file
                    CHECK(third_created_file->GetFileReferences().GetFileLinks().size() == 0);
                    CHECK(third_created_file->GetFileReferences().GetFileDependencies().size() == 1);
                    CHECK(third_created_file->GetFileReferences().GetFileDependencies().find(second_created_file_path) != third_created_file->GetFileReferences().GetFileDependencies().end());
                } 
            }

            AND_WHEN("One of those files is moved to another folder")
            {
                auto move_result = packetSystem.GetFileManager().MoveFile(second_created_file_path, move_file_dir);
                REQUIRE(move_result);

                auto move_path = move_result.value();

                AND_WHEN("These files are loaded")
                {
                    // Load the files
                    auto imported_file = file_loader.LoadFile(Packet::Hash(import_file_path));
                    auto first_created_file = file_loader.LoadFile(Packet::Hash(first_created_file_path));
                    auto second_created_file = file_loader.LoadFile(Packet::Hash(move_path));
                    auto third_created_file = file_loader.LoadFile(Packet::Hash(third_created_file_path));
                    REQUIRE(imported_file);
                    REQUIRE(first_created_file);
                    REQUIRE(second_created_file);
                    REQUIRE(third_created_file);

                    THEN("The involved references must be adjusted and pointing to the correct paths")
                    {
                        // Imported file
                        CHECK(imported_file->GetFileReferences().GetFileLinks().size() == 2);
                        CHECK(imported_file->GetFileReferences().GetFileDependencies().size() == 0);
                        CHECK(imported_file->GetFileReferences().GetFileLinks().find(first_created_file_path) != imported_file->GetFileReferences().GetFileLinks().end());
                        CHECK(imported_file->GetFileReferences().GetFileLinks().find(move_path) != imported_file->GetFileReferences().GetFileLinks().end());

                        // First created file
                        CHECK(first_created_file->GetFileReferences().GetFileLinks().size() == 1);
                        CHECK(first_created_file->GetFileReferences().GetFileDependencies().size() == 1);
                        CHECK(first_created_file->GetFileReferences().GetFileDependencies().find(import_file_path) != first_created_file->GetFileReferences().GetFileDependencies().end());
                        CHECK(first_created_file->GetFileReferences().GetFileLinks().find(move_path) != first_created_file->GetFileReferences().GetFileLinks().end());

                        // Second created file
                        CHECK(second_created_file->GetFileReferences().GetFileLinks().size() == 1);
                        CHECK(second_created_file->GetFileReferences().GetFileDependencies().size() == 2);
                        CHECK(second_created_file->GetFileReferences().GetFileDependencies().find(import_file_path) != second_created_file->GetFileReferences().GetFileDependencies().end());
                        CHECK(second_created_file->GetFileReferences().GetFileDependencies().find(first_created_file_path) != second_created_file->GetFileReferences().GetFileDependencies().end());
                        CHECK(second_created_file->GetFileReferences().GetFileLinks().find(third_created_file_path) != second_created_file->GetFileReferences().GetFileLinks().end());

                        // Third created file
                        CHECK(third_created_file->GetFileReferences().GetFileLinks().size() == 0);
                        CHECK(third_created_file->GetFileReferences().GetFileDependencies().size() == 1);
                        CHECK(third_created_file->GetFileReferences().GetFileDependencies().find(move_path) != third_created_file->GetFileReferences().GetFileDependencies().end());
                    }
                }
            }

            AND_WHEN("One of those files is copied")
            {
                auto copy_result = packetSystem.GetFileManager().CopyFile(second_created_file_path, copied_file_dir);
                REQUIRE(copy_result);

                auto copy_path = copy_result.value();

                AND_WHEN("These files are loaded")
                {
                    // Load the files
                    auto imported_file = file_loader.LoadFile(Packet::Hash(import_file_path));
                    auto first_created_file = file_loader.LoadFile(Packet::Hash(first_created_file_path));
                    auto second_created_file = file_loader.LoadFile(Packet::Hash(second_created_file_path));
                    auto third_created_file = file_loader.LoadFile(Packet::Hash(third_created_file_path));
                    auto copied_file = file_loader.LoadFile(Packet::Hash(copy_path));
                    REQUIRE(imported_file);
                    REQUIRE(first_created_file);
                    REQUIRE(second_created_file);
                    REQUIRE(third_created_file);
                    REQUIRE(copied_file);

                    THEN("The involved references must be the same, except by the ones affected by the copied file")
                    {
                        // Imported file
                        CHECK(imported_file->GetFileReferences().GetFileLinks().size() == 3);
                        CHECK(imported_file->GetFileReferences().GetFileDependencies().size() == 0);
                        CHECK(imported_file->GetFileReferences().GetFileLinks().find(first_created_file_path) != imported_file->GetFileReferences().GetFileLinks().end());
                        CHECK(imported_file->GetFileReferences().GetFileLinks().find(second_created_file_path) != imported_file->GetFileReferences().GetFileLinks().end());
                        CHECK(imported_file->GetFileReferences().GetFileLinks().find(copy_path) != imported_file->GetFileReferences().GetFileLinks().end());

                        // First created file
                        CHECK(first_created_file->GetFileReferences().GetFileLinks().size() == 2);
                        CHECK(first_created_file->GetFileReferences().GetFileDependencies().size() == 1);
                        CHECK(first_created_file->GetFileReferences().GetFileDependencies().find(import_file_path) != first_created_file->GetFileReferences().GetFileDependencies().end());
                        CHECK(first_created_file->GetFileReferences().GetFileLinks().find(second_created_file_path) != first_created_file->GetFileReferences().GetFileLinks().end());
                        CHECK(first_created_file->GetFileReferences().GetFileLinks().find(copy_path) != first_created_file->GetFileReferences().GetFileLinks().end());

                        // Second created file
                        CHECK(second_created_file->GetFileReferences().GetFileLinks().size() == 1);
                        CHECK(second_created_file->GetFileReferences().GetFileDependencies().size() == 2);
                        CHECK(second_created_file->GetFileReferences().GetFileDependencies().find(import_file_path) != second_created_file->GetFileReferences().GetFileDependencies().end());
                        CHECK(second_created_file->GetFileReferences().GetFileDependencies().find(first_created_file_path) != second_created_file->GetFileReferences().GetFileDependencies().end());
                        CHECK(second_created_file->GetFileReferences().GetFileLinks().find(third_created_file_path) != second_created_file->GetFileReferences().GetFileLinks().end());

                        // Third created file
                        CHECK(third_created_file->GetFileReferences().GetFileLinks().size() == 0);
                        CHECK(third_created_file->GetFileReferences().GetFileDependencies().size() == 1);
                        CHECK(third_created_file->GetFileReferences().GetFileDependencies().find(second_created_file_path) != third_created_file->GetFileReferences().GetFileDependencies().end());

                        // Copied file
                        CHECK(copied_file->GetFileReferences().GetFileLinks().size() == 0);
                        CHECK(copied_file->GetFileReferences().GetFileDependencies().size() == 2);
                        CHECK(copied_file->GetFileReferences().GetFileDependencies().find(import_file_path) != copied_file->GetFileReferences().GetFileDependencies().end());
                        CHECK(copied_file->GetFileReferences().GetFileDependencies().find(first_created_file_path) != copied_file->GetFileReferences().GetFileDependencies().end());
                    }
                }
            }

            AND_WHEN("One of those files is renamed")
            {
                auto rename_result = packetSystem.GetFileManager().RenameFile(second_created_file_path, renamed_file_name);
                REQUIRE(rename_result);

                auto rename_path = rename_result.value();

                AND_WHEN("These files are loaded")
                {
                    // Load the files
                    auto imported_file = file_loader.LoadFile(Packet::Hash(import_file_path));
                    auto first_created_file = file_loader.LoadFile(Packet::Hash(first_created_file_path));
                    auto second_created_file = file_loader.LoadFile(Packet::Hash(rename_path));
                    auto third_created_file = file_loader.LoadFile(Packet::Hash(third_created_file_path));
                    REQUIRE(imported_file);
                    REQUIRE(first_created_file);
                    REQUIRE(second_created_file);
                    REQUIRE(third_created_file);

                    THEN("The involved references must be adjusted and pointing to the correct paths")
                    {
                        // Imported file
                        CHECK(imported_file->GetFileReferences().GetFileLinks().size() == 2);
                        CHECK(imported_file->GetFileReferences().GetFileDependencies().size() == 0);
                        CHECK(imported_file->GetFileReferences().GetFileLinks().find(first_created_file_path) != imported_file->GetFileReferences().GetFileLinks().end());
                        CHECK(imported_file->GetFileReferences().GetFileLinks().find(rename_path) != imported_file->GetFileReferences().GetFileLinks().end());

                        // First created file
                        CHECK(first_created_file->GetFileReferences().GetFileLinks().size() == 1);
                        CHECK(first_created_file->GetFileReferences().GetFileDependencies().size() == 1);
                        CHECK(first_created_file->GetFileReferences().GetFileDependencies().find(import_file_path) != first_created_file->GetFileReferences().GetFileDependencies().end());
                        CHECK(first_created_file->GetFileReferences().GetFileLinks().find(rename_path) != first_created_file->GetFileReferences().GetFileLinks().end());

                        // Second created file
                        CHECK(second_created_file->GetFileReferences().GetFileLinks().size() == 1);
                        CHECK(second_created_file->GetFileReferences().GetFileDependencies().size() == 2);
                        CHECK(second_created_file->GetFileReferences().GetFileDependencies().find(import_file_path) != second_created_file->GetFileReferences().GetFileDependencies().end());
                        CHECK(second_created_file->GetFileReferences().GetFileDependencies().find(first_created_file_path) != second_created_file->GetFileReferences().GetFileDependencies().end());
                        CHECK(second_created_file->GetFileReferences().GetFileLinks().find(third_created_file_path) != second_created_file->GetFileReferences().GetFileLinks().end());

                        // Third created file
                        CHECK(third_created_file->GetFileReferences().GetFileLinks().size() == 0);
                        CHECK(third_created_file->GetFileReferences().GetFileDependencies().size() == 1);
                        CHECK(third_created_file->GetFileReferences().GetFileDependencies().find(rename_path) != third_created_file->GetFileReferences().GetFileDependencies().end());
                    }
                }
            }

            AND_WHEN("One of those files is deleted")
            {
                auto delete_result = packetSystem.GetFileManager().DeleteFile(second_created_file_path);
                REQUIRE(delete_result);

                AND_WHEN("The remaining files are loaded")
                {
                    // Load the files
                    auto imported_file = file_loader.LoadFile(Packet::Hash(import_file_path));
                    auto first_created_file = file_loader.LoadFile(Packet::Hash(first_created_file_path));
                    auto third_created_file = file_loader.LoadFile(Packet::Hash(third_created_file_path));
                    REQUIRE(imported_file);
                    REQUIRE(first_created_file);
                    REQUIRE(third_created_file);

                    THEN("Trying to load the deleted file must fail")
                    {
                        auto second_created_file = file_loader.LoadFile(Packet::Hash(second_created_file_path));
                        CHECK(second_created_file == nullptr);
                    }

                    AND_THEN("The files that the deleted one had a dependency must have their links removed, but the files who depends on the deleted one must keep their dependencies")
                    {
                        // Imported file
                        CHECK(imported_file->GetFileReferences().GetFileLinks().size() == 1);
                        CHECK(imported_file->GetFileReferences().GetFileDependencies().size() == 0);
                        CHECK(imported_file->GetFileReferences().GetFileLinks().find(first_created_file_path) != imported_file->GetFileReferences().GetFileLinks().end());

                        // First created file
                        CHECK(first_created_file->GetFileReferences().GetFileLinks().size() == 0);
                        CHECK(first_created_file->GetFileReferences().GetFileDependencies().size() == 1);
                        CHECK(first_created_file->GetFileReferences().GetFileDependencies().find(import_file_path) != first_created_file->GetFileReferences().GetFileDependencies().end());

                        // Third created file
                        CHECK(third_created_file->GetFileReferences().GetFileLinks().size() == 0);
                        CHECK(third_created_file->GetFileReferences().GetFileDependencies().size() == 1);
                        CHECK(third_created_file->GetFileReferences().GetFileDependencies().find(second_created_file_path) != third_created_file->GetFileReferences().GetFileDependencies().end());
                    }
                }
            }

            AND_WHEN("Dependencies from one of these files are redirected to another")
            {
                auto redirect_result = packetSystem.GetFileManager().RedirectFileDependencies(second_created_file_path, first_created_file_path);
                REQUIRE(redirect_result);

                AND_WHEN("The remaining files are loaded")
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

                    THEN("The dependencies must be moved to the other file")
                    {
                        // Imported file
                        CHECK(imported_file->GetFileReferences().GetFileLinks().size() == 2);
                        CHECK(imported_file->GetFileReferences().GetFileDependencies().size() == 0);
                        CHECK(imported_file->GetFileReferences().GetFileLinks().find(first_created_file_path) != imported_file->GetFileReferences().GetFileLinks().end());
                        CHECK(imported_file->GetFileReferences().GetFileLinks().find(second_created_file_path) != imported_file->GetFileReferences().GetFileLinks().end());

                        // First created file
                        CHECK(first_created_file->GetFileReferences().GetFileLinks().size() == 2);
                        CHECK(first_created_file->GetFileReferences().GetFileDependencies().size() == 1);
                        CHECK(first_created_file->GetFileReferences().GetFileDependencies().find(import_file_path) != first_created_file->GetFileReferences().GetFileDependencies().end());
                        CHECK(first_created_file->GetFileReferences().GetFileLinks().find(second_created_file_path) != first_created_file->GetFileReferences().GetFileLinks().end());
                        CHECK(first_created_file->GetFileReferences().GetFileLinks().find(third_created_file_path) != first_created_file->GetFileReferences().GetFileLinks().end());

                        // Second created file
                        CHECK(second_created_file->GetFileReferences().GetFileLinks().size() == 0);
                        CHECK(second_created_file->GetFileReferences().GetFileDependencies().size() == 2);
                        CHECK(second_created_file->GetFileReferences().GetFileDependencies().find(import_file_path) != second_created_file->GetFileReferences().GetFileDependencies().end());
                        CHECK(second_created_file->GetFileReferences().GetFileDependencies().find(first_created_file_path) != second_created_file->GetFileReferences().GetFileDependencies().end());

                        // Third created file
                        CHECK(third_created_file->GetFileReferences().GetFileLinks().size() == 0);
                        CHECK(third_created_file->GetFileReferences().GetFileDependencies().size() == 1);
                        CHECK(third_created_file->GetFileReferences().GetFileDependencies().find(first_created_file_path) != third_created_file->GetFileReferences().GetFileDependencies().end());
                    }
                }
            }
        }
    }
}