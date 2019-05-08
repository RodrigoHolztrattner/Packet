////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFile.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFile.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketFile::PacketFile(const FileHeader& _fileHeaderReference, const std::vector<uint8_t>& _fileIconDataReference) :
    m_FileHeaderReference(_fileHeaderReference), 
    m_FileIconDataReference(_fileIconDataReference)
{
	// Set the initial data
	// ...
}

PacketFile::~PacketFile()
{
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

const std::vector<uint8_t>& PacketFile::GetIconData() const
{
    return m_FileIconDataReference;
}

const FileHeader& PacketFile::GetFileHeader() const
{
    return m_FileHeaderReference;
}

const nlohmann::json& PacketFile::GetFileProperties() const
{
    return m_ParsedPropertiesData;
}

const FileReferences& PacketFile::GetFileReferences() const
{
    return m_ParsedFileReferences;
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
    m_ParsedFileReferences = FileReferences::CreateFromData(m_ReferencesData);
}