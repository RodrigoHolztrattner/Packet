////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileDefaultConverter.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileDefaultConverter.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketFileDefaultConverter::PacketFileDefaultConverter()
{
	// Set the initial data
	// ...
}

PacketFileDefaultConverter::~PacketFileDefaultConverter()
{
}

bool PacketFileDefaultConverter::ConvertExternalFile(
    std::vector<uint8_t>&& _original_file_data,
    std::vector<uint8_t>& _icon_data,
    std::vector<uint8_t>& _properties_data,
    std::vector<uint8_t>& _intermediate_data,
    std::vector<uint8_t>& _final_data) const
{
    // Just move the original data into the final data
    _final_data = std::move(_original_file_data);

    return true;
}

FileType PacketFileDefaultConverter::GetConversionFileType() const
{
    return "default";
}