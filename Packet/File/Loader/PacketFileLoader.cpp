////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileLoader.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketFileLoader::PacketFileLoader(std::filesystem::path _packet_path) :
    m_PacketPath(_packet_path)
{
}

PacketFileLoader::~PacketFileLoader()
{
}