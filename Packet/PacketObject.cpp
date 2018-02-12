////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketObject.h"
#include "PacketFileDataOperations.h"
#include "PacketStrings.h"

#include <fstream>
#include <experimental/filesystem>

Packet::PacketObject::PacketObject()
{
}

Packet::PacketObject::~PacketObject()
{
}

bool Packet::PacketObject::InitializeEmpty(std::string _packetName, uint32_t _maximumFragmentSize)
{
	// Verify the packet name extension?
	// not necessary

	// Set the pack name and the maximum fragment size
	m_PacketObjectName = _packetName;
	m_MaximumFragmentSize = _maximumFragmentSize;

	// Initialize the data for each sub system
	m_ObjectStructure.InitializeEmpty(_packetName);
	m_ObjectHashTable.InitializeEmpty();
	m_ObjectManager.InitializeEmpty(_packetName, _maximumFragmentSize);

	// Set the oppened file path
	m_OppenedFilePath = _packetName + PacketStrings::PacketExtensionType;

	return true;
}

bool Packet::PacketObject::InitializeFromFile(std::string _filePath)
{
	// Open the file
	std::ifstream file(_filePath, std::ios::binary);
	
	// Check if the file is good
	if (!file.good())
	{
		return false;
	}

	// Read the entire data
	std::vector<unsigned char> fileData = std::vector<unsigned char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	// Close the file
	file.close();

	// Set the initial data location
	uint32_t dataLocation = 0;

	// Get the pack name and the maximum fragment size
	m_PacketObjectName = PacketFileDataOperations::ReadFromData(fileData, dataLocation);
	m_MaximumFragmentSize = PacketFileDataOperations::ReadFromData<uint32_t>(fileData, dataLocation);
	
	// Get the data sizes
	uint32_t structureLocation = PacketFileDataOperations::ReadFromData<uint32_t>(fileData, dataLocation);
	uint32_t hashLocation = PacketFileDataOperations::ReadFromData<uint32_t>(fileData, dataLocation);
	uint32_t managerLocation = PacketFileDataOperations::ReadFromData<uint32_t>(fileData, dataLocation);

	// Process the data for each sub system
	m_ObjectStructure.InitializeFromData(fileData, dataLocation, m_PacketObjectName);
	m_ObjectHashTable.InitializeFromData(fileData, dataLocation);
	m_ObjectManager.InitializeFromData(fileData, dataLocation, m_PacketObjectName, m_MaximumFragmentSize);

	// Set the oppened file path
	m_OppenedFilePath = _filePath;

	return true;
}

bool Packet::PacketObject::SavePacketData()
{
	// Check if the file path is ok
	if (!m_OppenedFilePath.size())
	{
		return false;
	}

	// Save using the packet extension
	return SavePacketDataAux(m_OppenedFilePath);
}

bool Packet::PacketObject::SavePacketDataAux(std::string _filePath)
{
	// Our result vector 
	std::vector<unsigned char> resultVector;

	// Get the object datas
	std::vector<unsigned char> structureData = m_ObjectStructure.Serialize();
	std::vector<unsigned char> hashData = m_ObjectHashTable.Serialize();
	std::vector<unsigned char> managerData = m_ObjectManager.Serialize();

	// Process the data sizes
	uint32_t structureLocation = 0;
	uint32_t hashLocation = structureLocation + (uint32_t)structureData.size();
	uint32_t managerLocation = hashLocation + (uint32_t)hashData.size();

	// Our initial data location
	uint32_t currentDataLocation = 0;

	// Save the pack name and the maximum fragment size
	PacketFileDataOperations::SaveToData(resultVector, currentDataLocation, m_PacketObjectName);
	PacketFileDataOperations::SaveToData<uint32_t>(resultVector, currentDataLocation, m_MaximumFragmentSize);

	// Save the data sizes
	PacketFileDataOperations::SaveToData<uint32_t>(resultVector, currentDataLocation, structureLocation);
	PacketFileDataOperations::SaveToData<uint32_t>(resultVector, currentDataLocation, hashLocation);
	PacketFileDataOperations::SaveToData<uint32_t>(resultVector, currentDataLocation, managerLocation);

	// Append the data to the result vector
	resultVector.insert(resultVector.end(), structureData.begin(), structureData.end());
	resultVector.insert(resultVector.end(), hashData.begin(), hashData.end());
	resultVector.insert(resultVector.end(), managerData.begin(), managerData.end());

	// Create the file
	std::ofstream file(_filePath, std::ios::binary);

	// Check if the file is good
	if (!file.good())
	{
		return false;
	}

	// Copy the result data to the file
	std::copy(resultVector.begin(), resultVector.end(), std::ostreambuf_iterator<char>(file));

	// Close the file
	file.close();

	return true;
}

Packet::PacketObjectIterator Packet::PacketObject::GetIterator()
{
	return PacketObjectIterator(m_ObjectManager, m_ObjectStructure, m_ObjectHashTable);
}

Packet::PacketObjectManager* Packet::PacketObject::GetObjectManagerReference()
{
	return &m_ObjectManager;
}

Packet::PacketObjectStructure* Packet::PacketObject::GetObjectStructureReference()
{
	return &m_ObjectStructure;
}

Packet::PacketObjectHashTable* Packet::PacketObject::GetObjectHashTableReference()
{
	return &m_ObjectHashTable;
}