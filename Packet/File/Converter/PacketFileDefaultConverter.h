////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileDefaultConverter.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "../../PacketConfig.h"
#include "PacketFileConverter.h"

///////////////
// NAMESPACE //
///////////////

// Packet data explorer
PacketDevelopmentNamespaceBegin(Packet)

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFileDefaultConverter
////////////////////////////////////////////////////////////////////////////////
class PacketFileDefaultConverter : public PacketFileConverter
{

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFileDefaultConverter();
    ~PacketFileDefaultConverter();

//////////////////
// MAIN METHODS //
public: //////////

    // Convert an external file into an internal file, populating the output arrays with the file data
    bool ConvertExternalFile(
        std::vector<uint8_t>&& _original_file_data,
        std::vector<uint8_t>& _icon_data,
        std::vector<uint8_t>& _properties_data,
        std::vector<uint8_t>& _intermediate_data,
        std::vector<uint8_t>& _final_data) const final;

    // Return the type that this converter produces
    std::filesystem::path GetConversionFileExtension() const final;

///////////////
// VARIABLES //
private: //////

};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
