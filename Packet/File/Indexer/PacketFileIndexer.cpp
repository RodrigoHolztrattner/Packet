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