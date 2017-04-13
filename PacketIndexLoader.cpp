////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketIndexLoader.h"

Packet::PacketIndexLoader::PacketIndexLoader()
{
	// Set the initial data
	// ...
}

Packet::PacketIndexLoader::~PacketIndexLoader()
{
}

bool Packet::PacketIndexLoader::Initialize(std::string _rootFileName, PacketDirectory* _rootDirectory, PacketIndex& _indexData)
{
	// Save the root file name
	m_RootFileName = _rootFileName;

	// Set the index file name
	m_IndexFileName = m_RootFileName + IndexExtension;

	// Load the index data
	LoadIndex(_indexData, _rootDirectory);

	return true;
}

void Packet::PacketIndexLoader::Release()
{

}

void Packet::PacketIndexLoader::LoadIndex(PacketIndex& _indexData, PacketDirectory* _rootDirectory)
{
	// Open the index file
	std::ifstream indexFile(m_IndexFileName, std::ios::in | std::ios::binary);
	if (!indexFile.is_open())
	{
		// We didnt found a valid index file... using the dummy one...
		return;
	}

	// Load the total number of files
	ReadFromFile(indexFile, _indexData.totalNumberFiles);

	// Load the total number of folders
	ReadFromFile(indexFile, _indexData.totalNumberFolders);

	// The total of free list spaces
	uint32_t totalFileFreelist;
	uint32_t totalFolderFreelist;

	// Load the file freelist total
	ReadFromFile(indexFile, totalFileFreelist);

	// Load the folder freelist total
	ReadFromFile(indexFile, totalFolderFreelist);

	// Reserve space for all data
	_indexData.fileIdentifierFreelist.resize(totalFileFreelist);
	_indexData.folderIdentifierFreelist.resize(totalFolderFreelist);

	// Read the freelist data
	ReadStreamFromFile(indexFile, _indexData.fileIdentifierFreelist.data(), sizeof(uint32_t), totalFileFreelist);
	ReadStreamFromFile(indexFile, _indexData.folderIdentifierFreelist.data(), sizeof(uint32_t), totalFolderFreelist);

	// The directory and file info total data size
	uint32_t totalInfoData;

	// Read the total info data
	ReadFromFile(indexFile, totalInfoData);

	// Create the byte array that we will use to read the remaining data
	std::vector<unsigned char> byteArray;
	byteArray.resize(totalInfoData);

	// Read the byte array data
	ReadStreamFromFile(indexFile, byteArray.data(), sizeof(unsigned char), totalInfoData);

	// Close the index file
	indexFile.close();

	// Start reading the root folder
	uint32_t index = 0;
	ReadFolder(byteArray, index, _rootDirectory, _indexData);
}

void Packet::PacketIndexLoader::SaveIndex(PacketIndex& _indexData, PacketDirectory* _rootDirectory)
{
	// Create the new index file
	std::ofstream indexFile(m_IndexFileName, std::ios::out | std::ios::binary);
	if (!indexFile.is_open())
	{	
		// Problem creating the file!
		return;
	}

	// Save the total number of files
	WriteToFile(indexFile, _indexData.totalNumberFiles);

	// Save the total number of folders
	WriteToFile(indexFile, _indexData.totalNumberFolders);

	// The total of free list spaces
	uint32_t totalFileFreelist = _indexData.fileIdentifierFreelist.size();
	uint32_t totalFolderFreelist = _indexData.folderIdentifierFreelist.size();

	// Write the file freelist total
	WriteToFile(indexFile, totalFileFreelist);

	// Write the folder freelist total
	WriteToFile(indexFile, totalFolderFreelist);

	// Record all folder and file data into this byte array
	std::vector<unsigned char> byteArray;
	WriteFolder(byteArray, _rootDirectory);

	// The directory and file info total data size
	uint32_t totalInfoData = byteArray.size();

	// Write the total info data
	WriteToFile(indexFile, totalInfoData);

	// Write all the info data
	WriteStreamToFile(indexFile, byteArray.data(), sizeof(unsigned char), byteArray.size());

	// Close the index file
	indexFile.close();
}

void Packet::PacketIndexLoader::CreateIndexFile(PacketDirectory* _rootDirectory)
{
	// Create the new index file
	std::ofstream indexFile(m_IndexFileName, std::ios::out | std::ios::binary);

	// Set the initial data
	PacketDirectory dummyRootDirectory;
	uint32_t totalNumberFiles = 0;
	uint32_t totalNumberFolders = 0;
	uint32_t totalFileFreelist = 0;
	uint32_t totalFolderFreelist = 0;

	// Save the total number of files
	WriteToFile(indexFile, totalNumberFiles);

	// Save the total number of folders
	WriteToFile(indexFile, totalNumberFolders);

	// Serialize the root directory
	std::vector<unsigned char> serializedRootDirectory = _rootDirectory->Serialize();

	// Write the dummy root directory
	WriteStreamToFile(indexFile, serializedRootDirectory.data(), sizeof(unsigned char), serializedRootDirectory.size());
	
	// Close the index file
	indexFile.close();
}

void Packet::PacketIndexLoader::ReadFolder(std::vector<unsigned char>& _data, uint32_t& _index, PacketDirectory* _folder, PacketIndex& _indexData)
{
	// Deserialize the data
	_index = _folder->Deserialize(_data, _index);

	// Insert the folder into the references
	_indexData.folderIdentifierReferences[_folder->folderId] = _folder;

	// Read each child file info data
	for (int i = 0; i < _folder->childFileInfos.size(); i++)
	{
		// Read this file
		ReadFile(_data, _index, &_folder->childFileInfos[i], _indexData);
	}

	// Read each child folder data
	for (int i = 0; i < _folder->childFolders.size(); i++)
	{
		// Read this file
		ReadFolder(_data, _index, &_folder->childFolders[i], _indexData);
	}
}

void Packet::PacketIndexLoader::ReadFile(std::vector<unsigned char>& _data, uint32_t& _index, PacketFile* _fileInfo, PacketIndex& _indexData)
{
	// Deserialize the data
	_index = _fileInfo->Deserialize(_data, _index);

	// Insert the file into the references
	_indexData.fileIdentifierReferences[_fileInfo->fileId] = _fileInfo;
}

void Packet::PacketIndexLoader::WriteFolder(std::vector<unsigned char>& _data, PacketDirectory* _folder)
{
	// Serialize the data
	std::vector<unsigned char> serializedData = _folder->Serialize();

	// Append the data
	_data.insert(_data.end(), serializedData.begin(), serializedData.end());

	// Write each child file info data
	for (int i = 0; i < _folder->childFileInfos.size(); i++)
	{
		// Write this file
		WriteFile(_data, &_folder->childFileInfos[i]);
	}

	// Write each child folder data
	for (int i = 0; i < _folder->childFolders.size(); i++)
	{
		// Write this file
		WriteFolder(_data, &_folder->childFolders[i]);
	}
}

void Packet::PacketIndexLoader::WriteFile(std::vector<unsigned char>& _data, PacketFile* _fileInfo)
{
	// Serialize the data
	std::vector<unsigned char> serializedData = _fileInfo->Serialize();

	// Append the data
	_data.insert(_data.end(), serializedData.begin(), serializedData.end());
}