////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFile.h"

Packet::PacketFile::PacketFile()
{
	// Set the initial data
	// ...
}

Packet::PacketFile::~PacketFile()
{
}

std::vector<unsigned char> Packet::PacketFile::Serialize()
{
	// The main byte array
	std::vector<unsigned char> byteArray;

	// The serializer object
	Serialize::Serializer serializer(byteArray);

	// Pack the initial data
	serializer.PackData(fileName.GetString(), PacketString::MaxStringSize);
	serializer.PackData(fileExternalPath.GetString(), PacketString::MaxStringSize);
	serializer.PackData(fileId);

	return byteArray;
}

uint32_t Packet::PacketFile::Deserialize(std::vector<unsigned char>& _data, uint32_t _index)
{
	// The deserializer object
	Serialize::Deserializer deserializer(_data, _index);

	// Unpack the initial data
	deserializer.UnpackData(fileName.GetString(), PacketString::MaxStringSize);
	deserializer.UnpackData(fileExternalPath.GetString(), PacketString::MaxStringSize);
	deserializer.UnpackData(fileId);

	return deserializer.GetIndex();
}