////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketDirectory.h"
#include "PacketString.h"

Packet::PacketDirectory::PacketDirectory()
{
	// Set the initial data
	// ...
}

Packet::PacketDirectory::~PacketDirectory()
{
}

std::vector<unsigned char> Packet::PacketDirectory::Serialize()
{
	// The main byte array
	std::vector<unsigned char> byteArray;

	// The serializer object
	Serialize::Serializer serializer(byteArray);

	// Pack the initial data
	serializer.PackData(folderName.GetString(), PacketString::MaxStringSize);
	serializer.PackData(folderId);

	// Pack the total number of childs
	uint32_t totalChildFolders = childFolders.size();
	uint32_t totalChildFiles = childFileInfos.size();
	serializer.PackData(totalChildFolders);
	serializer.PackData(totalChildFiles);

	return byteArray;
}

uint32_t Packet::PacketDirectory::Deserialize(std::vector<unsigned char>& _data, uint32_t _index)
{
	// The deserializer object
	Serialize::Deserializer deserializer(_data, _index);

	// Unpack the initial data
	deserializer.UnpackData(folderName.GetString(), PacketString::MaxStringSize);
	deserializer.UnpackData(folderId);

	// Unpack the total number of childs
	uint32_t totalChildFolders;
	uint32_t totalChildFiles;
	deserializer.UnpackData(totalChildFolders);
	deserializer.UnpackData(totalChildFiles);

	// Resize our vectors
	childFolders.resize(totalChildFolders);
	childFileInfos.resize(totalChildFiles);

	return deserializer.GetIndex();
}