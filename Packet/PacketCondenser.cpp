////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketCondenser.h"
#include <filesystem>
#include <fstream>

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketCondenser::PacketCondenser()
{
	// Set the initial data
	// ...
}

PacketCondenser::~PacketCondenser()
{
}

std::vector<CondensedFileInfo> PacketCondenser::CondenseFolder(std::string _rootPath, uint32_t _currentCondenseFileCount, std::vector<std::pair<Hash, std::string>>& _fileInfos)
{
	// Out output vector
	std::vector<CondensedFileInfo> output;

	// Check if we have at last one file
	if (_fileInfos.size() == 0)
	{
		// There is nothing to be done
		return output;
	}

	// Get the root folder name
	auto rootFolderName = GetRootFolder(_rootPath);

	// Return a valid name for the condensed file
	auto GetCondensedFilename = [&]()
	{
		std::string temp = CondensedName;
		return rootFolderName.append(temp.append(std::to_string(_currentCondenseFileCount + output.size()).append(CondensedExtension)));
	};

	// The current condensed file info
	CondensedFileInfo currentCondensedFileInfo = {};
	currentCondensedFileInfo.filePath = GetCondensedFilename();

	// The current condensed file size and the current output write file
	uint64_t currentCondensedFileSize = 0;
	std::ofstream writeFile(currentCondensedFileInfo.filePath, std::ios::out);
	if (!writeFile.is_open())
	{
		// Error creating the file
		return std::vector<CondensedFileInfo>();
	}

	// For each file inside the given folder
	for (auto&[hash, filePath] : _fileInfos)
	{
		// Get the path to the file
		auto path = std::experimental::filesystem::path(filePath);

		// Get the file size
		auto fileSize = std::experimental::filesystem::file_size(path);

		// The condensed internal file info that we will use for this file
		CondensedFileInfo::InternalFileInfo internalFileInfo = {};
		internalFileInfo.hash = hash;
		internalFileInfo.size = uint64_t(fileSize);
		internalFileInfo.time = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());

		// Check if we need to generate another condensed file info
		if (currentCondensedFileSize + fileSize >= MaximumPackageSize)
		{
			// Insert it into the output vector
			output.push_back(currentCondensedFileInfo);

			// Create another condensed file info
			currentCondensedFileInfo = CondensedFileInfo();
			currentCondensedFileInfo.filePath = GetCondensedFilename();

			// Close the current file and write to a new one
			writeFile.close();
			writeFile = std::ofstream(currentCondensedFileInfo.filePath, std::ios::out);
			if (!writeFile.is_open())
			{
				// Error creating the file
				return std::vector<CondensedFileInfo>();
			}

			// Zero the current condensed file size
			currentCondensedFileSize = 0;
		}

		// Set the file location
		internalFileInfo.location = currentCondensedFileSize;

		// Open the target file and write it into the current condensed file
		{
			// Open the file
			std::ifstream file(filePath, std::ios::binary);
			if (!file.is_open())
			{
				// Error openning this file
				return std::vector<CondensedFileInfo>();
			}

			// Get the file data and close the file
			std::vector<char> buffer((unsigned int)fileSize);
			file.read(buffer.data(), fileSize);
			file.close();

			// Write the file data
			writeFile.write(buffer.data(), fileSize);

			// Increment the current condensed file size
			currentCondensedFileSize += fileSize;
		}

		// Set the file info
		currentCondensedFileInfo.fileInfos[currentCondensedFileInfo.totalNumberFiles] = internalFileInfo;

		// Increment the total number of files
		currentCondensedFileInfo.totalNumberFiles++;
	}

	// Close the current condensed file
	writeFile.close();

	// Insert the current condensed file into the output vector
	output.push_back(currentCondensedFileInfo);

	return output;
}