////////////////////////////////////////////////////////////////////////////////
// Filename: PacketCondensedFileLoader.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketCondensedFileLoader.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketCondensedFileLoader::PacketCondensedFileLoader(const PacketFileIndexer& _file_indexer)
{
	// Set the initial data
	// ...
}

PacketCondensedFileLoader::~PacketCondensedFileLoader()
{
}

std::unique_ptr<PacketFile> PacketCondensedFileLoader::LoadFile(Hash _file_hash) const
{
    return nullptr;
}

std::vector<uint8_t> PacketCondensedFileLoader::LoadFileRawData(Hash _file_hash) const
{
    return {};
}

std::optional<std::tuple<PacketFileHeader, std::vector<uint8_t>>> PacketCondensedFileLoader::LoadFileDataPart(Hash _file_hash, FilePart _file_part) const
{
    return std::nullopt;
}