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

std::vector<uint8_t> PacketFile::GenerateFileFromDataParts(
    HashPrimitive          _file_hash,
    FileType               _file_type,
    std::vector<uint8_t>&& _icon_data,
    std::vector<uint8_t>&& _properties_data,
    std::vector<uint8_t>&& _original_data, 
    std::vector<uint8_t>&& _intermediate_data,
    std::vector<uint8_t>&& _final_data,
    std::set<Path>&&       _files_that_depends_on_this,
    std::set<Path>&&       _file_dependencies)
{
    // Determine the positions
    FileDataPosition icon_position              = sizeof(PacketFileHeader);
    FileDataPosition properties_position        = icon_position + _icon_data.size();
    FileDataPosition original_data_position     = properties_position + _properties_data.size();
    FileDataPosition intermediate_data_position = original_data_position + _original_data.size();
    FileDataPosition final_data_position        = intermediate_data_position + _intermediate_data.size();
    FileDataPosition references_data_position   = final_data_position + _final_data.size();

    // Create the file references object and generate the vector data
    PacketFileReferences file_references = PacketFileReferences::CreateFromSets(
        std::move(_files_that_depends_on_this),
        std::move(_file_dependencies));
    std::vector<uint8_t> file_references_data = file_references.SaveIntoJSON();

    // Determine the total size
    FileDataSize file_total_size = sizeof(FileHeader)
        + _icon_data.size()
        + _properties_data.size()
        + _intermediate_data.size()
        + _final_data.size()
        + file_references_data.size();

    // Set the initial data for the file header
    FileHeader file_header                 = {};
    file_header.magic                      = FileMagic;
    file_header.version                    = PacketVersion;
    file_header.file_type                  = _file_type;
    file_header.file_hash                  = _file_hash;
    file_header.icon_position              = icon_position;
    file_header.properties_position        = properties_position;
    file_header.original_data_position     = original_data_position;
    file_header.intermediate_data_position = intermediate_data_position;
    file_header.final_data_position        = final_data_position;
    file_header.references_data_position   = references_data_position;

    // Compose the file data
    std::vector<uint8_t> file_data(file_total_size);
    *(reinterpret_cast<FileHeader*>(file_data.data())) = file_header;
    std::copy(_icon_data.begin(), _icon_data.end(), file_data.begin() + icon_position);
    std::copy(_properties_data.begin(), _properties_data.end(), file_data.begin() + properties_position);
    std::copy(_original_data.begin(), _original_data.end(), file_data.begin() + original_data_position);
    std::copy(_intermediate_data.begin(), _intermediate_data.end(), file_data.begin() + intermediate_data_position);
    std::copy(_final_data.begin(), _final_data.end(), file_data.begin() + final_data_position);
    std::copy(file_references_data.begin(), file_references_data.end(), file_data.begin() + references_data_position);

    return std::move(file_data);
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

PacketFileReferences& PacketFile::GetNonConstFileReferences()
{
    return m_ParsedFileReferences;
}

bool PacketFile::IsInternalFile() const
{
    return m_IsInternalFile;
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