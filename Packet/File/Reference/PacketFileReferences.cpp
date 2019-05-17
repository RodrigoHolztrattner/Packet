////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileReferences.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileReferences.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketFileReferences::PacketFileReferences()
{
	// Set the initial data
	// ...
}

PacketFileReferences::~PacketFileReferences()
{
}

void PacketFileReferences::ClearFileLinks()
{
    m_FileLinks.clear();
}

const std::set<Path>& PacketFileReferences::GetFileLinks() const
{
    return m_FileLinks;
}

// Return a set of all files that this one depends on
const std::set<Path>& PacketFileReferences::GetFileDependencies() const
{
    return m_FileDependencies;
}

PacketFileReferences PacketFileReferences::CreateFromData(const std::vector<uint8_t>& _data)
{
    return CreateFromJSON(_data.size() == 0 ? nlohmann::json::object() : nlohmann::json::from_bson(_data));
}

PacketFileReferences PacketFileReferences::CreateFromJSON(nlohmann::json _json)
{
    PacketFileReferences result;

    // Check if we have the necessary entries
    if (_json.count("FilesDependentOnThis") == 0 || _json.count("FileDependencies") == 0)
    {
        // Implicit move
        return result;
    }

    std::set<std::string> files_that_depends_on_this;
    std::set<std::string> file_dependencies;
    _json.at("FilesDependentOnThis").get_to(files_that_depends_on_this);
    _json.at("FileDependencies").get_to(file_dependencies);

    for (auto entry : files_that_depends_on_this)
    {
        result.m_FileLinks.insert(entry);
    }

    for (auto entry : file_dependencies)
    {
        result.m_FileDependencies.insert(entry);
    }

    // Implicit move
    return result;
}

PacketFileReferences PacketFileReferences::CreateFromSets(std::set<Path> _file_links,
                                           std::set<Path> _file_dependencies)
{
    PacketFileReferences result = {};
    result.m_FileLinks = std::move(_file_links);
    result.m_FileDependencies = std::move(_file_dependencies);
    return std::move(result);
}

nlohmann::json PacketFileReferences::SaveIntoJSON() const
{
    std::set<std::string> files_that_depends_on_this;
    std::set<std::string> file_dependencies;

    for (auto entry : m_FileLinks)
    {
        files_that_depends_on_this.insert(entry.to_raw_data_string());
    }

    for (auto entry : m_FileDependencies)
    {
        file_dependencies.insert(entry.to_raw_data_string());
    }

    return nlohmann::json{
    {"FilesDependentOnThis", files_that_depends_on_this},
    {"FileDependencies", file_dependencies} };
}

std::vector<uint8_t> PacketFileReferences::TransformIntoData(PacketFileReferences _references)
{
    return nlohmann::json::to_bson(_references.SaveIntoJSON());
}

void PacketFileReferences::AddFileLink(Path _new_link)
{
    m_FileLinks.insert(_new_link);
}

void PacketFileReferences::RemoveFileLink(Path _link)
{
    auto iter = m_FileLinks.find(_link);
    if (iter != m_FileLinks.end())
    {
        m_FileLinks.erase(iter);
    }
}