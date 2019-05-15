////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileConverter.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "../../PacketConfig.h"

///////////////
// NAMESPACE //
///////////////

// Packet data explorer
PacketDevelopmentNamespaceBegin(Packet)

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFileConverter
////////////////////////////////////////////////////////////////////////////////
class PacketFileConverter
{

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFileConverter();
    virtual ~PacketFileConverter();

//////////////////
// MAIN METHODS //
public: //////////

    // Convert an external file into an internal file, populating the output arrays with the file data
    virtual bool ConvertExternalFile(
        std::vector<uint8_t>&& _original_file_data,
        std::vector<uint8_t>& _icon_data,
        std::vector<uint8_t>& _properties_data,
        std::vector<uint8_t>& _intermediate_data,
        std::vector<uint8_t>& _final_data) = 0;

    // Return the type that this converter produces
    virtual FileType GetConversionFileType() const = 0;

///////////////
// VARIABLES //
private: //////

};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
