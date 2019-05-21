////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileIndexer.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileIndexer.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketFileIndexer::PacketFileIndexer(std::filesystem::path _packet_path) :
    m_PacketPath(_packet_path)
{
	// Set the initial data
	// ...
}

PacketFileIndexer::~PacketFileIndexer()
{
}

void PacketFileIndexer::SetAuxiliarObjects(const PacketFileLoader* _file_loader)
{
    m_FileLoaderPtr = _file_loader;
}

void PacketFileIndexer::RegisterFileModificationCallback(FileModificationCallback _callback)
{
    m_FileModificationCallbacks.push_back(_callback);
}

std::vector<std::pair<Path, std::set<Path>>> PacketFileIndexer::GetMissingDependenciesInfo() const
{
    return {};
}