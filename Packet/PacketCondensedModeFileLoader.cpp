////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketCondensedModeFileLoader.h"
#include <filesystem>

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketCondensedModeFileLoader::PacketCondensedModeFileLoader(std::string _packetManifestDirectory, PacketLogger* _logger) : PacketFileLoader(_packetManifestDirectory, _logger)
{
	// Load the packet data
	m_PackedDataLoaded = ReadPacketData(_packetManifestDirectory);
	if (!m_PackedDataLoaded)
	{
		// Error!
		m_Logger->LogError(std::string("Error reading the packed folder data on: ").append(_packetManifestDirectory).c_str());
	}

	// Process the packet data
	ProcessPacketData();

	// Set the initial data
	// ...
}

PacketCondensedModeFileLoader::~PacketCondensedModeFileLoader()
{
	// For each condensed file reader
	for (auto& reader : m_FileReaders)
	{
		// If the file is active, delete (implicity close it)
		if (reader.file != nullptr)
		{
			// Delete it
			delete reader.file;
		}
	}
}

bool PacketCondensedModeFileLoader::FileExist(Hash _fileHash)
{
	// Check if we have this file inside our mapped files
	if (m_MappedInternalFileInfos.find(_fileHash) != m_MappedInternalFileInfos.end())
	{
		return true;
	}

	return false;
}

uint64_t PacketCondensedModeFileLoader::GetFileSize(Hash _fileHash)
{
	// If the file exist
	if (FileExist(_fileHash))
	{
		// Get the file info and return the size
		auto iter = m_MappedInternalFileInfos.find(_fileHash);
		return iter->second.info.size;
	}

	// Invalid file
	return 0;
}

bool PacketCondensedModeFileLoader::GetFileData(uint8_t* _dataOut, uint64_t _bufferSize, Hash _fileHash)
{
	// If the file doesn't exist
	if (!FileExist(_fileHash))
	{
		return false;
	}

	// Get the file info
	MappedInternalFileInfo& mappedFileInfo = m_MappedInternalFileInfos.find(_fileHash)->second;

	// Check if the file reader is valid
	if (mappedFileInfo.readerIndex < 0 || mappedFileInfo.readerIndex >= m_FileReaders.size())
	{
		return false;
	}

	// Get a short variable to the file object
	auto& file = *m_FileReaders[mappedFileInfo.readerIndex].file;

	// Read the file data
	file.read((char*)_dataOut, _bufferSize);
	if (!file)
	{
		// Error reading the file
		m_Logger->LogError(std::string("Error reading the file: ")
			.append(_fileHash.GetPath())
			.append(", only ")
			.append(std::to_string(file.gcount()))
			.append(" bytes could be read!")
			.c_str());

		return false;
	}

	return true;
}

bool PacketCondensedModeFileLoader::ConstructPacket()
{
	// We can't construct the packet structure on this mode!
	m_Logger->LogError(std::string("We can't construct the packet structure on this mode!").c_str());

	return false;
}

bool PacketCondensedModeFileLoader::ReadPacketData(std::string _packetManifestDirectory)
{
	// Get the root folder name
	auto rootFolderName = GetRootFolder(_packetManifestDirectory);

	// Setup the condensed info file name
	std::string condensedInfoName = rootFolderName;
	condensedInfoName.append(CondensedInfoName);
	condensedInfoName.append(CondensedInfoExtension);

	// Open the file and check if we are ok to proceed
	std::ifstream file(condensedInfoName, std::ios::binary);
	if (!file.is_open())
	{
		// Error openning the file!
		m_Logger->LogError(std::string("Error trying to open the packed data on: ").append(_packetManifestDirectory).c_str());

		return false;
	}

	// Read the condensed header
	file.read((char*)&m_CondensedFileHeader, sizeof(CondensedHeaderInfo));

	// Allocate memory for all file infos
	m_CondensedFileInfos.resize(m_CondensedFileHeader.totalInfos);

	// Read the condensed file infos
	file.read((char*)m_CondensedFileInfos.data(), sizeof(CondensedFileInfo) * m_CondensedFileHeader.totalInfos);

	// Close the file
	file.close();

	return true;
}

void PacketCondensedModeFileLoader::ProcessPacketData()
{
	// For each file info
	for (auto& fileInfo : m_CondensedFileInfos)
	{
		// The condensed file header we will be using
		CondensedFileReader fileReader = {};
		fileReader.filePath = fileInfo.filePath;
		fileReader.totalNumberFiles = fileInfo.totalNumberFiles;

		// Open the file
		fileReader.file = new std::ifstream(fileInfo.filePath, std::ios::binary);
		if (!fileReader.file->is_open())
		{
			// Error openning the file!
			m_Logger->LogError(std::string("Error reading a packet info data! File: ").append(fileInfo.filePath).c_str());
			m_PackedDataLoaded = false;
			delete fileReader.file;

			return;
		}

		// For each internal file info
		for (unsigned int i = 0; i < fileInfo.totalNumberFiles; i++)
		{
			// Get the internal file info reference
			auto& internalFileInfo = fileInfo.fileInfos[i];

			// Setup the mapped internal file info
			MappedInternalFileInfo mappedFileInfo = {};
			mappedFileInfo.info = internalFileInfo;
			mappedFileInfo.readerIndex = uint16_t(m_FileReaders.size());

			// Insert it
			m_MappedInternalFileInfos.insert({ internalFileInfo.hash.GetHashValue(), mappedFileInfo });
		}

		// Insert the file reader into our vector
		m_FileReaders.push_back(fileReader);
	}

	// Log info
	m_Logger->LogInfo(std::string("Found a total of ")
		.append(std::to_string(m_MappedInternalFileInfos.size()))
		.append(" file infos when reading the packet object!")
		.c_str());
}

std::vector<Hash> PacketCondensedModeFileLoader::GetFileHashesThatNeedUpdate(std::string _packetManifestDirectory)
{
	// The output vector
	std::vector<Hash> out;

	// Check if our packet data was loaded
	if (!m_PackedDataLoaded)
	{
		// We need to load the packet data first
		return out;
	}

	// Get the root folder name
	auto rootFolderName = GetRootFolder(_packetManifestDirectory);

	// Setup the condensed info file name
	std::string condensedInfoName = rootFolderName;
	condensedInfoName.append(CondensedInfoName);
	condensedInfoName.append(CondensedInfoExtension);

	// Open the file and check if we are ok to proceed
	std::ifstream file(condensedInfoName, std::ios::binary);
	if (!file.is_open())
	{
		// Error openning the file!
		m_Logger->LogError(std::string("Error trying to open the packed data on: ").append(_packetManifestDirectory).c_str());

		return out;
	}

	// The data header info
	CondensedHeaderInfo headerInfo;

	// Read the condensed header
	file.read((char*)&headerInfo, sizeof(CondensedHeaderInfo));

	// The condensed file infos
	std::vector<CondensedFileInfo> condensedFileInfos;

	// Allocate memory for all file infos
	condensedFileInfos.resize(headerInfo.totalInfos);

	// Read the condensed file infos
	file.read((char*)condensedFileInfos.data(), sizeof(CondensedFileInfo) * headerInfo.totalInfos);

	// Close the file
	file.close();

	// Check if the read header is newer the our current one
	if(m_CondensedFileHeader.saveTime >= headerInfo.saveTime 
		|| m_CondensedFileHeader.majorVersion > headerInfo.majorVersion
		|| (m_CondensedFileHeader.majorVersion == headerInfo.majorVersion && m_CondensedFileHeader.minorVersion > headerInfo.minorVersion))
	{
		// Our current header is newer or equal to the given one
		return out;
	}

	// Ok the new header is newer then ours, lets update //

	// Now for each condensed file info
	for (auto& condensedFileInfo : condensedFileInfos)
	{
		// For each internal file inside this condensed info
		for (unsigned int i = 0; i < condensedFileInfo.totalNumberFiles; i++)
		{
			// Get the internal file info
			auto& internalFileInfo = condensedFileInfo.fileInfos[i];

			// Check if we have this file info
			auto iter = m_MappedInternalFileInfos.find(internalFileInfo.hash);
			if (iter == m_MappedInternalFileInfos.end())
			{
				// We don't have this file, add its hash into the output vector
				out.push_back(internalFileInfo.hash);
			}
			// We have the file
			else
			{
				// Get the file info
				auto& currentInternalFileInfo = iter->second.info;

				// Compare the written date
				if (currentInternalFileInfo.time < internalFileInfo.time)
				{
					// The newer file has a newert written date, insert its hash into the output vector
					out.push_back(internalFileInfo.hash);
				}
			}
		}
	}

	return out;
}