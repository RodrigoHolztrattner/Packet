////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketReferenceManager.h"
#include "../PacketFile.h"
#include "../Loader/PacketFileLoader.h"
#include "../Saver/PacketFileSaver.h"
#include "../Importer/PacketFileImporter.h"
#include <filesystem>
#include <fstream>

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketReferenceManager::PacketReferenceManager()
{
	// Set the initial data
	// ...
}

PacketReferenceManager::~PacketReferenceManager()
{
}

void PacketReferenceManager::SetAuxiliarObjects(const PacketFileLoader* _file_loader, const PacketFileSaver* _file_saver)
{
    m_FileLoaderPtr = _file_loader;
    m_FileSaverPtr = _file_saver;
}

bool PacketReferenceManager::RegisterDependenciesForFile(std::set<Path> _file_dependencies, Path _file_path) const
{
    // For each dependency
    for (auto& dependency : _file_dependencies)
    {
        // Add a reference link
        if (!AddReferenceLink(_file_path, dependency))
        {
            // This should never happen if we follow all procedures correctly
            return false;
        }
    }

    return true;
}

bool PacketReferenceManager::AddReferenceLink(Path _file_path, Path _reference) const
{
    auto file_references_opt = m_FileLoaderPtr->LoadFileDataPart(Hash(_reference), FilePart::ReferencesData);
    if (!file_references_opt)
    {
        // This should never happen if we follow all procedures correctly
        return false;
    }
    auto [header, references_data] = file_references_opt.value();
    auto references = PacketFileReferences::CreateFromData(references_data);

    // Add the link
    references.AddFileLink(_file_path);

    // Transform back to data
    references_data = PacketFileReferences::TransformIntoData(references);

    // Save this file reference data
    return m_FileSaverPtr->SaveFile(header, FilePart::ReferencesData, std::move(references_data));
}

bool PacketReferenceManager::RemoveReferenceLink(Path _file_path, Path _reference) const
{
    auto file_references_opt = m_FileLoaderPtr->LoadFileDataPart(Hash(_reference), FilePart::ReferencesData);
    if (!file_references_opt)
    {
        // This should never happen if we follow all procedures correctly
        return false;
    }
    auto [header, references_data] = file_references_opt.value();
    auto references = PacketFileReferences::CreateFromData(references_data);

    // Remove the link
    references.RemoveFileLink(_file_path);

    // Transform back to data
    references_data = PacketFileReferences::TransformIntoData(references);

    // Save this file reference data
    return m_FileSaverPtr->SaveFile(header, FilePart::ReferencesData, std::move(references_data));
}

bool PacketReferenceManager::RedirectLinks(std::set<Path> _referenced_files_paths, Path _old_path, Path _new_path) const
{
    // TODO: Create a method that will only update the references instead having to load the entire file to save it again

    // For each referenced file
    bool operation_result = true;
    for (auto& referenced_file_path : _referenced_files_paths)
    {
        // Load the target file
        auto referenced_file_data = m_FileLoaderPtr->LoadFileRawData(Hash(referenced_file_path));
        if (referenced_file_data.size() == 0)
        {
            // This should never happen if we follow all procedures correctly
            operation_result = false;
            continue;
        }

        // Substitute all path references for this file data
        SubstituteAllPathReferences(referenced_file_data, _old_path, _new_path);

        // Create the file from the updated data
        auto updated_file = PacketFile::CreateFileFromRawData(std::move(referenced_file_data), referenced_file_path);
        if (!updated_file)
        {
            return false;
        }

        // Write the file data into a system file
        if (!m_FileSaverPtr->SaveFile(std::move(updated_file), SaveOperation::Overwrite))
        {
            // This should never happen if we follow all procedures correctly
            operation_result = false;
            continue;
        }
    }

    return operation_result;
}

std::pair<std::set<Path>, std::set<Path>> PacketReferenceManager::RetrieveDependencyDiffFromOriginalFile(const std::unique_ptr<PacketFile>& _file) const
{
    // Get the original references
    auto original_references_data_opt = m_FileLoaderPtr->LoadFileDataPart(Hash(_file->GetFileHeader().GetOriginalPath()), FilePart::ReferencesData);
    if (!original_references_data_opt)
    {
        assert(false && "Trying to retrieve the original file references but its data is invalid!");
        return { {}, {} };
    }
    auto [header, original_references_data] = original_references_data_opt.value();
    auto original_dependencies = PacketFileReferences::CreateFromData(original_references_data).GetFileDependencies();

    // Get the current dependencies
    auto current_dependencies = _file->GetFileReferences().GetFileDependencies();

    // Determine the delete diff
    std::set<Path> delete_diff;
    std::set_difference(
        original_dependencies.begin(),
        original_dependencies.end(),
        current_dependencies.begin(),
        current_dependencies.end(),
        std::inserter(delete_diff, delete_diff.begin()));

    // Determine the add diff
    std::set<Path> add_diff;
    std::set_difference(
        current_dependencies.begin(),
        current_dependencies.end(),
        original_dependencies.begin(),
        original_dependencies.end(),
        std::inserter(add_diff, add_diff.begin()));

    return { std::move(delete_diff), std::move(add_diff) };
}

void PacketReferenceManager::SubstituteAllPathReferences(std::vector<uint8_t>& _file_data, Path _lookup_path, Path _new_path) const
{
    // TODO: This can be easily parallelized

    // Get the lookup string and the new path string
    std::string lookup_string   = _lookup_path.string();
    std::string new_path_string = _new_path.string();

    std::string string_data(_file_data.begin(), _file_data.end());

    std::string::size_type n = 0;
    while ((n = string_data.find(lookup_string, n)) != std::string::npos)
    {
        string_data.replace(n, lookup_string.size(), new_path_string);
        n += new_path_string.size();
    }

    _file_data = std::vector<uint8_t>(string_data.begin(), string_data.end());
}