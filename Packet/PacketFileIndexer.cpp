////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileIndexer.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileIndexer.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketFileIndexer::PacketFileIndexer(const FileHeader& _fileHeaderReference, const std::vector<uint8_t>& _fileIconDataReference) :
    m_FileHeaderReference(_fileHeaderReference), 
    m_FileIconDataReference(_fileIconDataReference)
{
	// Set the initial data
	// ...
}

PacketFileIndexer::~PacketFileIndexer()
{
}

const std::vector<uint8_t>& PacketFileIndexer::GetOriginalData() const
{
    return m_OriginalData;
}

const std::vector<uint8_t>& PacketFileIndexer::GetIntermediateData() const
{
    return m_IntermediateData;
}

const std::vector<uint8_t>& PacketFileIndexer::GetFinalData() const
{
    return m_FinalData;
}

const std::vector<uint8_t>& PacketFileIndexer::GetIconData() const
{
    return m_FileIconDataReference;
}

const FileHeader& PacketFileIndexer::GetFileHeader() const
{
    return m_FileHeaderReference;
}

const nlohmann::json& PacketFileIndexer::GetFileProperties() const
{
    return m_ParsedPropertiesData;
}

const FileReferences& PacketFileIndexer::GetFileReferences() const
{
    return m_ParsedFileReferences;
}

void PacketFileIndexer::SetData(std::vector<uint8_t>&& _propertiesData,
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

void PacketFileIndexer::ParseProperties()
{
    m_ParsedPropertiesData = nlohmann::json::parse(m_PropertiesData.begin(), m_PropertiesData.end());
}

void PacketFileIndexer::ParseReferences()
{
    m_ParsedFileReferences = FileReferences::CreateFromData(m_ReferencesData);
}