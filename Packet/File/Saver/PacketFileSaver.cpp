////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileSaver.h"
#include "../Indexer/PacketPlainFileIndexer.h"
#include "../Reference/PacketReferenceManager.h"
#include "../Loader/PacketFileLoader.h"
#include "../PacketFile.h"
#include "../PacketFileHeader.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketFileSaver::PacketFileSaver(const PacketFileIndexer& _file_indexer, 
                                 const PacketReferenceManager& _reference_manager,
                                 const PacketFileLoader& _file_loader, 
                                 std::filesystem::path _packet_path) :
    m_PacketPath(_packet_path), 
    m_ReferenceManager(_reference_manager), 
    m_FileLoader(_file_loader), 
    m_FileIndexer(_file_indexer)
{
}

PacketFileSaver::~PacketFileSaver()
{
}

bool PacketFileSaver::SaveFile(std::unique_ptr<PacketFile> _file, SaveOperation _operation) const
{
    // Check if the file is valid
    if (!_file || _file->IsExternalFile())
    {
        return false;
    }

    /*
        - To duplicate or rename a file, we must: Duplicate original file + Save duplicated one + Delete old file
        - To copy a file, we must:                Duplicate original file + Save duplicated one
        - To create a new file, we must:          Save the new file
        - Update existing file, we must:          Save updated file
    */

    // If the original file doesn't exist, we are creating a new one
    // Its dependencies must be set and no link is necessary, this is a new file and
    // no other file must depend on it now
    if (_operation == SaveOperation::Create)
    {
        if (_file->GetFileReferences().GetFileLinks().size() != 0)
        {
            return false;
        }

        // Go through all dependencies, for each one open the target file and register a link
        m_ReferenceManager.RegisterDependenciesForFile(_file->GetFileReferences().GetFileDependencies(), _file->GetFileHeader().GetPath());
    }
    // We are copying a file, so we must set its dependencies and do not update its
    // links, also its link data must be empty, this is considered a new file but must
    // inherit its original file dependencies
    else if (_operation == SaveOperation::Copy)
    {
        if (_file->GetFileReferences().GetFileLinks().size() != 0)
        {
            return false;
        }

        // Go through all dependencies, for each one open the target file and register a link
        m_ReferenceManager.RegisterDependenciesForFile(_file->GetFileReferences().GetFileDependencies(), _file->GetFileHeader().GetPath());
    }
    // We are overwriting/editing the original file, so we must verify if its dependencies 
    // are different and update them (by doing this we must update their links), also for 
    // each link we have we must update the target resource to point to the new file path
    else if (_operation == SaveOperation::Overwrite)
    {
        // Get the dependency diff
        auto [delete_dependencies, add_dependencies] = m_ReferenceManager.RetrieveDependencyDiffFromOriginalFile(_file);

        // For each dependency that must be deleted
        for (auto& dependency : delete_dependencies)
        {
            m_ReferenceManager.RemoveReferenceLink(dependency, _file->GetFileHeader().GetPath());
        }

        // For each dependency that must be added
        for (auto& dependency : add_dependencies)
        {
            m_ReferenceManager.AddReferenceLink(dependency, _file->GetFileHeader().GetPath());
        }

        // For each link, open the target file and make it depend on this file new path, 
        // also substitute the occurrences inside the target file data
        m_ReferenceManager.RedirectLinks(_file->GetFileReferences().GetFileLinks(), _file->GetFileHeader().GetOriginalPath(), _file->GetFileHeader().GetPath());
    }
    // We are moving or renaming the original file, so no need to verify its dependencies
    // because they didn't change, but for each link we have we must update the target 
    // resource to point to the new file path
    else if (_operation == SaveOperation::Rename || _operation == SaveOperation::Move)
    {
        // For each link, open the target file and make it depend on this file new path, 
        // also substitute the occurrences inside the target file data
        m_ReferenceManager.RedirectLinks(_file->GetFileReferences().GetFileLinks(), _file->GetFileHeader().GetOriginalPath(), _file->GetFileHeader().GetPath());
    }
    // If this is the result of modifying the reference we shouldn't need to do anything 
    // with them, it could cause a infinite loop or make us access a file that is being
    // wrote
    else if (_operation == SaveOperation::ReferenceUpdate)
    {
        // Do nothing
        // ...
    }
    else
    {
        throw "What?";
    }

    return SaveFileHelper(std::move(_file));
}

bool PacketFileSaver::SaveFile(const PacketFileHeader& _file_header, FilePart _file_part, std::vector<uint8_t>&& _file_data_part) const
{
    // The header cannot be updated directly
    if (_file_part == FilePart::Header)
    {
        return false;
    }

    // Load the file
    auto file = m_FileLoader.LoadFile(Hash(_file_header.GetPath()));
    if (!file)
    {
        return false;
    }

    // Do not update if the file is an external file
    if (file->IsExternalFile())
    {
        return false;
    }

    // Update the data part
    if (!file->UpdateFilePart(_file_part, std::move(_file_data_part)))
    {
        return false;
    }

    // Save the file
    return SaveFile(std::move(file), _file_part == FilePart::ReferencesData ? SaveOperation ::ReferenceUpdate : SaveOperation::Overwrite);
}

bool PacketFileSaver::SaveFileHelper(std::unique_ptr<PacketFile> _file) const
{
    // Check if the file is valid
    if (!_file)
    {
        return false;
    }

    // Get the file path
    auto file_path = _file->GetFileHeader().GetPath();

    /*
    // Get the current data sizes
    FileDataSize icon_data_size         = _file->GetIconData().size();
    FileDataSize properties_data_size   = _file->GetPropertiesData().size();
    FileDataSize original_data_size     = _file->GetOriginalData().size();
    FileDataSize intermediate_data_size = _file->GetIntermediateData().size();
    FileDataSize final_data_size        = _file->GetFinalData().size();
    FileDataSize references_data_size   = _file->GetReferencesData().size();
    */

    // Transform the file into raw data
    auto file_raw_data = PacketFile::CreateRawDataFromFile(std::move(_file));
    if (file_raw_data.size() == 0)
    {
        return false;
    }

    /*
    // Get an editable file header from the raw data
    auto* file_header = PacketFileHeader::GetHeaderDataPtr(file_raw_data);
    if (!file_header)
    {
        return false;
    }
    */
    
    /*
    // Update the header positions
    file_header->icon_position              = sizeof(PacketFileHeader::FileHeaderData);
    file_header->properties_position        = file_header->icon_position + icon_data_size;
    file_header->original_data_position     = file_header->properties_position + properties_data_size;
    file_header->intermediate_data_position = file_header->original_data_position + original_data_size;
    file_header->final_data_position        = file_header->intermediate_data_position + intermediate_data_size;
    file_header->references_data_position   = file_header->final_data_position + final_data_size;
    */

    // Determine the system path this file must be saved on
    auto file_output_path = MergeSystemPathWithFilePath(m_PacketPath, file_path);

    // Open the file and check if we are ok to proceed
    std::ofstream system_file(file_output_path, std::ios::binary);
    if (!system_file.is_open())
    {
        // Error opening the file!
        return false;
    }

    // Write the file data
    system_file.write(reinterpret_cast<char*>(file_raw_data.data()), file_raw_data.size() * sizeof(uint8_t));

    // Close the file, operation success
    system_file.close();

    return true;
}