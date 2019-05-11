////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketReferenceManager.h"
#include "File/PacketFile.h"
#include "PacketFileLoader.h"
#include "PacketFileImporter.h"
#include <filesystem>
#include <fstream>

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketReferenceManager::PacketReferenceManager(const PacketFileLoader& _file_loader, const PacketFileImporter& _file_importer) :
    m_FileLoader(_file_loader), 
    m_FileImporter(_file_importer)
{
	// Set the initial data
	// ...
}

PacketReferenceManager::~PacketReferenceManager()
{
}

bool PacketReferenceManager::AddReferenceLink(Path _file_path, Path _reference) const
{
    // Load the target file
    auto referenced_file = m_FileLoader.LoadFile(Hash(_file_path));
    if (!referenced_file)
    {
        // This should never happen if we follow all procedures correctly
        return false;
    }

    // Get the file references (non const reference variable)
    PacketFileReferences& file_references = referenced_file->GetNonConstFileReferences();

    // Add the new entry
    file_references.AddFileLink(_reference);

    // Retrieve the edited file raw data
    auto file_raw_data = PacketFile::TransformFileIntoRawData(std::move(referenced_file));

    // Write the file data into a system file
    if (!m_FileImporter.WriteFileDataIntoInternalFile(_file_path, std::move(file_raw_data)))
    {
        // This should never happen if we follow all procedures correctly
        return false;
    }

    return true;
}

bool PacketReferenceManager::RemoveReferenceLink(Path _file_path, Path _reference) const
{
    // Load the target file
    auto referenced_file = m_FileLoader.LoadFile(Hash(_file_path));
    if (!referenced_file)
    {
        // This should never happen if we follow all procedures correctly
        return false;
    }

    // Get the file references (non const reference variable)
    PacketFileReferences& file_references = referenced_file->GetNonConstFileReferences();

    // Remove the entry
    file_references.RemoveFileLink(_reference);

    // Retrieve the edited file raw data
    auto file_raw_data = PacketFile::TransformFileIntoRawData(std::move(referenced_file));

    // Write the file data into a system file
    if (!m_FileImporter.WriteFileDataIntoInternalFile(_file_path, std::move(file_raw_data)))
    {
        // This should never happen if we follow all procedures correctly
        return false;
    }

    return true;
}

bool PacketReferenceManager::RedirectLinks(std::set<Path> _referenced_files_paths, Path _old_path, Path _new_path) const
{
    // TODO: Create a method that will only update the references instead having to load the entire file to save it again

    // For each referenced file
    bool operation_result = true;
    for (auto& referenced_file_path : _referenced_files_paths)
    {
        // Load the target file
        auto referenced_file_data = m_FileLoader.LoadFileRawData(Hash(referenced_file_path));
        if (referenced_file_data.size() == 0)
        {
            // This should never happen if we follow all procedures correctly
            operation_result = false;
            continue;
        }

        // Substitute all path references for this file data
        SubstituteAllPathReferences(referenced_file_data, _old_path, _new_path);

        // Write the file data into a system file
        if (!m_FileImporter.WriteFileDataIntoInternalFile(referenced_file_path, std::move(referenced_file_data)))
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
    auto original_references_data_opt = m_FileLoader.LoadFileDataPart(Hash(_file->GetFileHeader().GetOriginalPath()), FilePart::ReferencesData);
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
        delete_diff);

    // Determine the add diff
    std::set<Path> add_diff;
    std::set_difference(
        current_dependencies.begin(),
        current_dependencies.end(),
        original_dependencies.begin(),
        original_dependencies.end(),
        add_diff);

    return { std::move(delete_diff), std::move(add_diff) };
}

#include <string_view>

void PacketReferenceManager::SubstituteAllPathReferences(std::vector<uint8_t>& _file_data, Path _lookup_path, Path _new_path) const
{
    // TODO: This can be easily parallelized

    // Get the lookup string and the new path string
    std::string lookup_string = _lookup_path.String();
    std::string new_path_string = _new_path.String();

    std::string string_data(_file_data.begin(), _file_data.end());

    std::string::size_type n = 0;
    while ((n = string_data.find(lookup_string, n)) != std::string::npos)
    {
        string_data.replace(n, lookup_string.size(), new_path_string);
        n += new_path_string.size();
    }

    _file_data = std::vector<uint8_t>(string_data.begin(), string_data.end());
}