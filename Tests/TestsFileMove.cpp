#include <catch2/catch.hpp>
#include <string>
#include <vector>
#include <fstream>

#include "../Packet/Packet.h"
#include "MyResource.h"
#include "MyFactory.h"
#include "HelperMethods.h"
#include "HelperDefines.h"

void ValidateMove(Packet::System& _packet_system,
                  Packet::FileIndexer& _file_indexer,
                  const Packet::FileLoader& _file_loader,
                  std::string _original_path,
                  std::string _new_path)
{
    // Load the file
    auto initial_file = _file_loader.LoadFile(Packet::Hash(_original_path));
    REQUIRE(initial_file != nullptr);

    auto move_result = _packet_system.GetFileManager().MoveFile(_original_path, _new_path);
    
    THEN("The move must be successful")
    {
        REQUIRE(move_result);
    }

    AND_THEN("The old path must not be valid anymore (if it was different from the original one)")
    {
        if (_original_path != move_result.value().string())
        {
            CHECK(std::filesystem::exists(ResourceDirectory + "/" + _original_path) == false);
        }
    }

    AND_THEN("The file must be located on the new path")
    {
        CHECK(std::filesystem::exists(ResourceDirectory + "/" + move_result.value().string()) == true);
    }

    AND_THEN("The original file path must not be indexed and the new path must be (if they are different)")
    {
        if (_original_path == move_result.value().string())
        {
            CHECK(_file_indexer.IsFileIndexed(Packet::Hash(move_result.value())) == true);
        }
        else
        {
            CHECK(_file_indexer.IsFileIndexed(Packet::Hash(_original_path)) == false);
            CHECK(_file_indexer.IsFileIndexed(Packet::Hash(move_result.value())) == true);
        }
    }

    AND_THEN("The file data must be identical")
    {
        // Load the file from the new path
        auto moved_file = _file_loader.LoadFile(Packet::Hash(move_result.value()));
        REQUIRE(moved_file);

        CHECK(initial_file->GetFileHeader().GetVersion() == moved_file->GetFileHeader().GetVersion());
        CHECK(initial_file->GetFileHeader().GetFileSize() == moved_file->GetFileHeader().GetFileSize());
        CHECK(initial_file->GetFileHeader().GetFileType().string() == moved_file->GetFileHeader().GetFileType().string());
        CHECK(initial_file->GetIconData() == moved_file->GetIconData());
        CHECK(initial_file->GetPropertiesData() == moved_file->GetPropertiesData());
        CHECK(initial_file->GetOriginalData() == moved_file->GetOriginalData());
        CHECK(initial_file->GetIntermediateData() == moved_file->GetIntermediateData());
        CHECK(initial_file->GetFinalData() == moved_file->GetFinalData());
        CHECK(initial_file->GetReferencesData() == moved_file->GetReferencesData());
        CHECK(initial_file->GetFileReferences().GetFileLinks() == moved_file->GetFileReferences().GetFileLinks());
        CHECK(initial_file->GetFileReferences().GetFileDependencies() == moved_file->GetFileReferences().GetFileDependencies());
        CHECK(initial_file->IsExternalFile() == moved_file->IsExternalFile());
    }
}

SCENARIO("Internal files can be moved to any location inside the packet path", "[move]")
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

        WHEN("The packet file is moved to a different path")
        {
            // Setup the path we will move the file
            auto move_target_path = "Images";

            ValidateMove(packetSystem,
                         file_indexer,
                         file_loader,
                         file_path,
                         move_target_path);
        }

        AND_WHEN("The packet file is moved to the same folder but with a different name (some sort of renaming)")
        {
            // Setup the path we will move the file
            auto same_folder_move_path = "Sounds";

            ValidateMove(packetSystem,
                         file_indexer,
                         file_loader,
                         file_path,
                         same_folder_move_path);
        }

        AND_WHEN("The packet file is moved to the same folder with the same name")
        {
            // Setup the path we will move the file
            auto same_folder_move_path = file_path;

            ValidateMove(packetSystem,
                         file_indexer,
                         file_loader,
                         file_path,
                         same_folder_move_path);
        }
    }
}