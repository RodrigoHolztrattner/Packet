////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketEditModeFileLoader.h"
#include <filesystem>
#include <iostream>
#include <fstream>

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketEditModeFileLoader::PacketEditModeFileLoader(std::string _packetFolderPath) : PacketFileLoader(_packetFolderPath)
{
}

PacketEditModeFileLoader::~PacketEditModeFileLoader()
{
}

bool PacketEditModeFileLoader::FileExist(Hash _fileHash)
{
	// Get the path
	auto filePath = std::experimental::filesystem::path(_fileHash.GetPath().String());

	// Check if the file exist and is valid
	if (std::experimental::filesystem::exists(filePath) && !std::experimental::filesystem::is_directory(filePath))
	{
		// Valid file
		return true;
	}
	else
	{
		// File doesn't exist or ir a path to a directory
		return false;
	}
}

uint64_t PacketEditModeFileLoader::GetFileSize(Hash _fileHash)
{
	// Get the path
	auto filePath = std::experimental::filesystem::path(_fileHash.GetPath().String());

	// Check if the file exist and is valid
	if (std::experimental::filesystem::exists(filePath) && !std::experimental::filesystem::is_directory(filePath))
	{
		// Valid file
		return uint64_t(std::experimental::filesystem::file_size(filePath));
	}

	// Invalid file
	return 0;
}

bool PacketEditModeFileLoader::GetFileData(uint8_t* _dataOut, uint32_t _bufferSize, Hash _fileHash)
{
	// Open the file and check if we are ok to proceed
	std::ifstream file(_fileHash.GetPath().String(), std::ios::binary);
	if (!file.is_open())
	{
		// Error openning the file!
		return false;
	}

	// Read the file data
	if (file.read((char*)_dataOut, _bufferSize))
	{
		return true;
	}

	return false;
}

bool PacketEditModeFileLoader::ConstructPacket()
{
	////////////////////
	// INITIALIZATION //
	////////////////////

	// Scan the packet tree and file/folder info
	m_Scanner.Scan(m_PacketFolderPath);

	// Check for duplicated hashes
	auto hashCollisions = m_Scanner.CheckForHashCollisions();
	if (hashCollisions.size() != 0)
	{
		std::cout << "Found files that will cause hash collisions: " << std::endl;

		// For each hash collision
		for (auto& collision : hashCollisions)
		{
			std::cout << " - " << collision << std::endl;
		}

		std::cout << std::endl;

		return false;
	}

	// Get the file tree info
	std::vector<std::pair<std::string, std::vector<std::pair<Hash, std::string>>>> fileTree = m_Scanner.GetFileTreeHashInfos();

	//////////////
	// CONDENSE //
	//////////////

	// For each folder info
	std::vector<CondensedFileInfo> condensedFileInfos;
	for (auto& folderInfo : fileTree)
	{
		// Process this folder info
		auto condensedFiles = m_PacketCondenser.CondenseFolder(m_PacketFolderPath, condensedFileInfos.size(), folderInfo.second);

		// Merge the vectors
		condensedFileInfos.insert(condensedFileInfos.end(), condensedFiles.begin(), condensedFiles.end());
	}

	//////////////////////
	// CREATE INFO FILE //
	//////////////////////
	{
		// Get the root folder name
		auto rootFolderName = GetRootFolder(m_PacketFolderPath);

		// Setup the condensed info file name
		std::string condensedInfoName = rootFolderName;
		condensedInfoName.append(CondensedInfoName);
		condensedInfoName.append(CondensedInfoExtension);

		// Create the file and check if we are ok to proceed
		std::ofstream file(condensedInfoName, std::ios::binary);
		if (!file.is_open())
		{
			// Error openning the file!
			return false;
		}

		// Setup the condensed header info
		CondensedHeaderInfo condensedHeader;
		condensedHeader.totalInfos = condensedFileInfos.size();
		condensedHeader.saveTime = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
		condensedHeader.majorVersion = CondensedMajorVersion;
		condensedHeader.minorVersion = CondensedMinorVersion;

		// Save the header info
		file.write((char*)&condensedHeader, sizeof(CondensedHeaderInfo));

		// Save the file infos
		file.write((char*)condensedFileInfos.data(), sizeof(CondensedFileInfo) * condensedFileInfos.size());

		// Close the file
		file.close();
	}

	return true;
}