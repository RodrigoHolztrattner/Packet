#include <catch2/catch.hpp>
#include <string>
#include <vector>
#include <fstream>

#include "../Packet/Packet.h"
#include "MyResource.h"
#include "MyFactory.h"
#include "HelperMethods.h"
#include "HelperDefines.h"

void ValidateRename(Packet::System& _packet_system,
                    Packet::FileIndexer& _file_indexer,
                    const Packet::FileLoader& _file_loader,
                    std::string _original_path,
                    std::string _new_name)
{
    // Load the file
    auto initial_file = _file_loader.LoadFile(Packet::Hash(_original_path));
    REQUIRE(initial_file != nullptr);

    auto rename_result = _packet_system.GetFileManager().RenameFile(_original_path, _new_name);

    // Determine if the filenames are equal to prevent doing unnecessary tests
    bool file_names_are_equal = std::filesystem::path(_original_path).stem() == std::filesystem::path(_new_name).stem();

    THEN("The rename must be successful")
    {
        REQUIRE(rename_result);
    }

    if (!file_names_are_equal)
    {
        AND_THEN("The old path must not be valid anymore")
        {
            REQUIRE(std::filesystem::exists(ResourceDirectory + "/" + _original_path) == false);
        }
    }

    AND_THEN("The file must be located on the new path")
    {
        REQUIRE(std::filesystem::exists(ResourceDirectory + "/" + rename_result.value().string()) == true);
    }

    AND_THEN("The original file path must not be indexed and the new path must be")
    {
        if (!file_names_are_equal)
        {
            REQUIRE(_file_indexer.IsFileIndexed(Packet::Hash(_original_path)) == false);
        }
        REQUIRE(_file_indexer.IsFileIndexed(Packet::Hash(rename_result.value())) == true);
    }

    AND_THEN("The file data must be identical")
    {
        // Load the file from the new path
        auto renamed_file = _file_loader.LoadFile(Packet::Hash(rename_result.value()));
        REQUIRE(renamed_file);

        REQUIRE(initial_file->GetFileHeader().GetVersion() == renamed_file->GetFileHeader().GetVersion());
        REQUIRE(initial_file->GetFileHeader().GetFileSize() == renamed_file->GetFileHeader().GetFileSize());
        REQUIRE(initial_file->GetFileHeader().GetFileType().string() == renamed_file->GetFileHeader().GetFileType().string());
        REQUIRE(initial_file->GetIconData() == renamed_file->GetIconData());
        REQUIRE(initial_file->GetPropertiesData() == renamed_file->GetPropertiesData());
        REQUIRE(initial_file->GetOriginalData() == renamed_file->GetOriginalData());
        REQUIRE(initial_file->GetIntermediateData() == renamed_file->GetIntermediateData());
        REQUIRE(initial_file->GetFinalData() == renamed_file->GetFinalData());
        REQUIRE(initial_file->GetReferencesData() == renamed_file->GetReferencesData());
        REQUIRE(initial_file->GetFileReferences().GetFileLinks() == renamed_file->GetFileReferences().GetFileLinks());
        REQUIRE(initial_file->GetFileReferences().GetFileDependencies() == renamed_file->GetFileReferences().GetFileDependencies());
        REQUIRE(initial_file->IsExternalFile() == renamed_file->IsExternalFile());
    }
}

SCENARIO("Internal files can be renamed to any valid filename", "[rename]")
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

        WHEN("The packet file is renamed to a different name")
        {
            // Setup the new file name
            auto new_file_name = "renamed_imported_file";

            ValidateRename(packetSystem,
                           file_indexer,
                           file_loader,
                           file_path,
                           new_file_name);
        }

        AND_WHEN("The packet file renamed to the same name")
        {
            // Setup the new file name
            auto new_file_name = std::filesystem::path(file_path).stem().string();

            ValidateRename(packetSystem,
                           file_indexer,
                           file_loader,
                           file_path,
                           new_file_name);
        }

        AND_WHEN("The operation must fail if it's being renamed to an invalid name (there is already an existing file with that name)")
        {
            // Setup the path we will import the second file
            auto second_file_path = "Sounds/second_imported_file.pckfile";

            // Import the second file
            bool second_import_result = file_importer.ImportExternalFile(ExternalFilePath, second_file_path);
            REQUIRE(second_import_result == true);

            // Setup the new file name
            auto new_file_name = std::filesystem::path(second_file_path).stem().string();

            // Try to rename
            auto rename_result = packetSystem.GetFileManager().RenameFile(file_path, new_file_name);

            AND_THEN("The rename result must be false")
            {
                REQUIRE(rename_result == std::nullopt);
            }
        }
    }
}