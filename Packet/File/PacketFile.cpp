////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFile.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFile.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketFile::PacketFile()
{
}

PacketFile::PacketFile(PacketFileHeader _fileHeaderReference, const std::vector<uint8_t> _fileIconDataReference, bool _is_internal_file) :
    m_FileHeader(_fileHeaderReference), 
    m_FileIconData(_fileIconDataReference), 
    m_IsInternalFile(_is_internal_file)
{
	// Set the initial data
	// ...
}

PacketFile::~PacketFile()
{
}

std::unique_ptr<PacketFile> PacketFile::CreateFileFromRawData(std::vector<uint8_t>&& _file_data)
{
    std::unique_ptr<PacketFile> result_file;

    // Get the file header
    auto file_header = PacketFileHeader::CreateFromRawData(_file_data);
    if (!file_header)
    {
        // Invalid file header or size!
        return nullptr;
    }

    // Check the version
    // ...

    // Determine the data locations
    auto icon_data_begin = _file_data.begin() + file_header->GetDataPosition(FilePart::IconData);
    auto icon_data_end = _file_data.begin() + (file_header->GetDataPosition(FilePart::PropertiesData) - file_header->GetDataPosition(FilePart::IconData));
    auto properties_data_begin = _file_data.begin() + file_header->GetDataPosition(FilePart::PropertiesData);
    auto properties_data_end = _file_data.begin() + (file_header->GetDataPosition(FilePart::OriginalData) - file_header->GetDataPosition(FilePart::PropertiesData));
    auto original_data_begin = _file_data.begin() + file_header->GetDataPosition(FilePart::OriginalData);
    auto original_data_end = _file_data.begin() + (file_header->GetDataPosition(FilePart::IntermediateData) - file_header->GetDataPosition(FilePart::OriginalData));
    auto intermediate_data_begin = _file_data.begin() + file_header->GetDataPosition(FilePart::IntermediateData);
    auto intermediate_data_end = _file_data.begin() + (file_header->GetDataPosition(FilePart::FinalData) - file_header->GetDataPosition(FilePart::IntermediateData));
    auto final_data_begin = _file_data.begin() + file_header->GetDataPosition(FilePart::FinalData);
    auto final_data_end = _file_data.begin() + (file_header->GetDataPosition(FilePart::ReferencesData) - file_header->GetDataPosition(FilePart::FinalData));
    auto references_data_begin = _file_data.begin() + file_header->GetDataPosition(FilePart::ReferencesData);
    auto references_data_end = _file_data.end();

    // Setup the data vectors
    std::vector<uint8_t> icon_data = std::vector<uint8_t>(icon_data_begin, icon_data_end);
    std::vector<uint8_t> properties_data = std::vector<uint8_t>(properties_data_begin, properties_data_end);
    std::vector<uint8_t> original_data = std::vector<uint8_t>(original_data_begin, original_data_end);
    std::vector<uint8_t> intermediate_data = std::vector<uint8_t>(intermediate_data_begin, intermediate_data_end);
    std::vector<uint8_t> final_data = std::vector<uint8_t>(final_data_begin, final_data_end);
    std::vector<uint8_t> references_data = std::vector<uint8_t>(final_data_begin, final_data_end);

    // Initialize the result file
    result_file = std::make_unique<PacketFile>(file_header, std::move(icon_data), true);

    // Set the data
    result_file->SetData(std::move(properties_data), 
                         std::move(original_data), 
                         std::move(intermediate_data), 
                         std::move(final_data), 
                         std::move(references_data));

    return result_file;
}

std::unique_ptr<PacketFile> PacketFile::DuplicateFile(const std::unique_ptr<PacketFile>& _file)
{
    std::unique_ptr<PacketFile> result_file = std::make_unique<PacketFile>();

    // Duplicate all the data
    result_file->m_FileHeader           = _file->m_FileHeader;
    result_file->m_FileIconData         = _file->m_FileIconData;
    result_file->m_PropertiesData       = _file->m_PropertiesData;
    result_file->m_OriginalData         = _file->m_OriginalData;
    result_file->m_IntermediateData     = _file->m_IntermediateData;
    result_file->m_FinalData            = _file->m_FinalData;
    result_file->m_ReferencesData       = _file->m_ReferencesData;
    result_file->m_ParsedPropertiesData = _file->m_ParsedPropertiesData;
    result_file->m_ParsedFileReferences = _file->m_ParsedFileReferences;
    result_file->m_IsInternalFile       = _file->m_IsInternalFile;

    return result_file;
}

/*
std::vector<uint8_t> PacketFile::CreateRawDataFromFile(std::unique_ptr<PacketFile> _file)
{
    if (!_file)
    {
        return {};
    }

    HashPrimitive file_hash = _file->m_FileHeader.GetHash();
    FileType file_type = _file->m_FileHeader.GetFileType();
    std::vector<uint8_t> icon_data = std::move(_file->m_FileIconData);
    std::vector<uint8_t> properties_data = std::move(_file->m_PropertiesData);
    std::vector<uint8_t> original_data = std::move(_file->m_OriginalData);
    std::vector<uint8_t> intermediate_data = std::move(_file->m_IntermediateData);
    std::vector<uint8_t> final_data = std::move(_file->m_FinalData);
    std::set<Path> files_that_depends_on_this = _file->m_ParsedFileReferences.GetFileLinks();
    std::set<Path> file_dependencies = _file->m_ParsedFileReferences.GetFileDependencies();

    return GenerateInternalFileData(
        file_hash,
        file_type,
        std::move(icon_data),
        std::move(properties_data),
        std::move(original_data),
        std::move(intermediate_data),
        std::move(final_data),
        std::move(files_that_depends_on_this),
        std::move(file_dependencies));
}
*/

std::unique_ptr<PacketFile> PacketFile::GenerateFileFromData(
    Path                   _file_path,
    FileType               _file_type,
    std::vector<uint8_t>&& _icon_data,
    std::vector<uint8_t>&& _properties_data,
    std::vector<uint8_t>&& _original_data, 
    std::vector<uint8_t>&& _intermediate_data,
    std::vector<uint8_t>&& _final_data,
    std::set<Path>&&       _file_dependencies)
{
    // Create an empty file header data object and set its initial values
    PacketFileHeader file_header;
    file_header.SetFileType(_file_type);
    file_header.SetPath(_file_path);

    // Create the file references object and generate the vector data
    PacketFileReferences file_references = PacketFileReferences::CreateFromSets(
        {},
        std::move(_file_dependencies));
    std::vector<uint8_t> file_references_data = PacketFileReferences::TransformIntoData(file_references);

    // Create a new empty file
    std::unique_ptr<PacketFile> new_file = std::make_unique<PacketFile>();

    // Set the new file header
    new_file->m_FileHeader = file_header;

    // Set its data
    new_file->UpdateFilePart(FilePart::IconData, std::move(_icon_data));
    new_file->UpdateFilePart(FilePart::PropertiesData, std::move(_properties_data));
    new_file->UpdateFilePart(FilePart::OriginalData, std::move(_original_data));
    new_file->UpdateFilePart(FilePart::IntermediateData, std::move(_intermediate_data));
    new_file->UpdateFilePart(FilePart::FinalData, std::move(_final_data));
    new_file->UpdateFilePart(FilePart::ReferencesData, std::move(file_references_data));

    return std::move(new_file);
}

/*
bool PacketFile::BreakFileIntoDatas(
    std::unique_ptr<PacketFile> _file,
    std::vector<uint8_t>&       _icon_data,
    std::vector<uint8_t>&       _properties_data,
    std::vector<uint8_t>&       _original_data,
    std::vector<uint8_t>&       _intermediate_data,
    std::vector<uint8_t>&       _final_data,
    PacketFileReferences&       _file_references_parsed)
{
    if (!_file)
    {
        return false;
    }

    _icon_data              = std::move(_file->m_FileIconData);
    _properties_data        = std::move(_file->m_PropertiesData);
    _original_data          = std::move(_file->m_OriginalData);
    _intermediate_data      = std::move(_file->m_IntermediateData);
    _final_data             = std::move(_file->m_FinalData);
    _file_references_parsed = std::move(_file->m_ParsedFileReferences);

    return true;
}
*/

const std::vector<uint8_t>& PacketFile::GetPropertiesData() const
{
    return m_PropertiesData;
}

const std::vector<uint8_t>& PacketFile::GetOriginalData() const
{
    return m_OriginalData;
}

const std::vector<uint8_t>& PacketFile::GetIntermediateData() const
{
    return m_IntermediateData;
}

const std::vector<uint8_t>& PacketFile::GetFinalData() const
{
    return m_FinalData;
}

const std::vector<uint8_t>& PacketFile::GetReferencesData() const
{
    return m_ReferencesData;
}

const std::vector<uint8_t>& PacketFile::GetIconData() const
{
    return m_FileIconData;
}

const PacketFileHeader& PacketFile::GetFileHeader() const
{
    return m_FileHeader;
}

const nlohmann::json& PacketFile::GetFileProperties() const
{
    return m_ParsedPropertiesData;
}

const PacketFileReferences& PacketFile::GetFileReferences() const
{
    return m_ParsedFileReferences;
}

bool PacketFile::IsInternalFile() const
{
    return m_IsInternalFile;
}

void PacketFile::ClearFileLinks()
{
    // Clear the links and update the data
    m_ParsedFileReferences.ClearFileLinks();
    m_ReferencesData = PacketFileReferences::TransformIntoData(m_ParsedFileReferences);
}

bool PacketFile::UpdateFilePart(FilePart _file_part, std::vector<uint8_t>&& _data)
{
    // Cannot update the header
    if (_file_part == FilePart::Header)
    {
        return false;
    }

    FileDataSize old_data_size = 0;
    switch (_file_part)
    {
        case FilePart::IconData:
        {
            old_data_size = m_FileIconData.size();
            m_FileIconData = _data;
            break;
        }
        case FilePart::PropertiesData:
        {
            old_data_size = m_PropertiesData.size();
            m_PropertiesData = _data;
            ParseProperties();
            break;
        }
        case FilePart::OriginalData:
        {
            old_data_size = m_OriginalData.size();
            m_OriginalData = _data;
            break;
        }
        case FilePart::IntermediateData:
        {
            old_data_size = m_IntermediateData.size();
            m_IntermediateData = _data;
            break;
        }
        case FilePart::FinalData:
        {
            old_data_size = m_FinalData.size();
            m_FinalData = _data;
            break;
        }
        case FilePart::ReferencesData:
        {
            old_data_size = m_ReferencesData.size();
            m_ReferencesData = _data;
            ParseReferences();
            break;
        }
        default:
        {
            return false;
        }
    }
    
    // Determine the size difference
    FileDataSize size_difference = _data.size() - old_data_size;

    // Update the header file size
    m_FileHeader.SetFileSize(m_FileHeader.GetFileSize() + size_difference);

    return true;
}

void PacketFile::SetData(std::vector<uint8_t>&& _propertiesData,
                         std::vector<uint8_t>&& _originalData,
                         std::vector<uint8_t>&& _intermediateData,
                         std::vector<uint8_t>&& _finalData,
                         std::vector<uint8_t>&& _referencesData)
{
    // Remaining file data
    m_PropertiesData   = std::move(_propertiesData);
    m_OriginalData     = std::move(_originalData);
    m_IntermediateData = std::move(_intermediateData);
    m_FinalData        = std::move(_finalData);
    m_ReferencesData   = std::move(_referencesData);

    // Parse our new data
    ParseProperties();
    ParseReferences();
}

void PacketFile::ParseProperties()
{
    m_ParsedPropertiesData = nlohmann::json::parse(m_PropertiesData.begin(), m_PropertiesData.end());
}

void PacketFile::ParseReferences()
{
    m_ParsedFileReferences = PacketFileReferences::CreateFromData(m_ReferencesData);
}