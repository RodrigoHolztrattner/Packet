#include <catch2/catch.hpp>
#include <string>
#include <vector>
#include <fstream>

#include "../Packet/Packet.h"
#include "MyResource.h"
#include "MyFactory.h"
#include "HelperMethods.h"
#include "HelperDefines.h"

void ValidateCopy(Packet::System& _packet_system,
                  Packet::FileIndexer& _file_indexer,
                  const Packet::FileLoader& _file_loader,
                  std::string _original_path,
                  std::string _new_path)
{
    // Load the file
    auto initial_file = _file_loader.LoadFile(Packet::Hash(_original_path));
    REQUIRE(initial_file != nullptr);

    auto copy_result = _packet_system.GetFileManager().CopyFile(_original_path, _new_path);

    THEN("The copy must be successful")
    {
        REQUIRE(copy_result);
    }

    AND_THEN("The old path must still be valid")
    {
        CHECK(std::filesystem::exists(ResourceDirectory + "/" + _original_path) == true);
    }

    AND_THEN("The new path must contain a new file")
    {
        CHECK(std::filesystem::exists(ResourceDirectory + "/" + copy_result.value().string()) == true);
    }

    AND_THEN("The original and new file paths must be indexed")
    {
        CHECK(_file_indexer.IsFileIndexed(Packet::Hash(_original_path)) == true);
        CHECK(_file_indexer.IsFileIndexed(Packet::Hash(copy_result.value())) == true);
    }

    AND_THEN("The file data must be identical, except by the path")
    {
        // Load the file from the new path
        auto copied_file = _file_loader.LoadFile(Packet::Hash(copy_result.value()));
        REQUIRE(copied_file);

        CHECK(initial_file->GetFileHeader().GetVersion() == copied_file->GetFileHeader().GetVersion());
        CHECK(initial_file->GetFileHeader().GetFileSize() == copied_file->GetFileHeader().GetFileSize());
        CHECK(initial_file->GetFileHeader().GetPath().string() != copied_file->GetFileHeader().GetPath().string());
        CHECK(initial_file->GetIconData() == copied_file->GetIconData());
        CHECK(initial_file->GetPropertiesData() == copied_file->GetPropertiesData());
        CHECK(initial_file->GetOriginalData() == copied_file->GetOriginalData());
        CHECK(initial_file->GetIntermediateData() == copied_file->GetIntermediateData());
        CHECK(initial_file->GetFinalData() == copied_file->GetFinalData());
        CHECK(initial_file->GetReferencesData() == copied_file->GetReferencesData());
        CHECK(initial_file->GetFileReferences().GetFileLinks() == copied_file->GetFileReferences().GetFileLinks());
        CHECK(initial_file->GetFileReferences().GetFileDependencies() == copied_file->GetFileReferences().GetFileDependencies());
        CHECK(initial_file->IsExternalFile() == copied_file->IsExternalFile());
    }
}

SCENARIO("Internal files can be copied to any location inside the packet path", "[copy]")
{
    // Setup the playground
    SetupResourcePlayground();

    GIVEN("A packet system initialized on edit mode and an already existing packet file")
    {
        Packet::System packetSystem;
        bool packet_initialize_result = packetSystem.Initialize(Packet::OperationMode::Plain, ResourceDirectory);
        REQUIRE(packet_initialize_result == true);

        // Request the file importer, indexer and loader
        auto & file_importer = packetSystem.GetFileManager().GetFileImporter();
        auto & file_indexer = packetSystem.GetFileManager().GetFileIndexer();
        auto & file_loader = packetSystem.GetFileManager().GetFileLoader();

        // Setup the path we will import the file
        auto file_dir = "Sounds/";

        // Import the file
        auto import_result = file_importer.ImportExternalFile(ExternalFilePath, file_dir);
        REQUIRE(import_result);
        auto file_path = import_result.value().string();

        WHEN("The packet file is copied to a different path")
        {
            // Setup the path we will copy the file
            auto copy_target_path = "Images";

            ValidateCopy(packetSystem,
                         file_indexer,
                         file_loader,
                         file_path,
                         copy_target_path);
        }

        AND_WHEN("The packet file is copied to the same folder")
        {
            // Setup the path we will copy the file
            auto same_folder_copy_path = "Sounds";

            ValidateCopy(packetSystem,
                         file_indexer,
                         file_loader,
                         file_path,
                         same_folder_copy_path);
        }

        AND_WHEN("The packet file is copied to the same folder with the same name")
        {
            // Setup the path we will copy the file
            auto same_folder_copy_path = file_path;

            ValidateCopy(packetSystem,
                         file_indexer,
                         file_loader,
                         file_path,
                         same_folder_copy_path);
        }
    }
}