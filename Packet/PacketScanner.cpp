////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketScanner.h"
#include <filesystem>
#include <functional>
#include <map>

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketScanner::PacketScanner()
{
	// Set the initial data
	m_RootFolderNode = nullptr;
}

PacketScanner::~PacketScanner()
{
}

void PacketScanner::Scan(std::string _packetDirectory)
{
	// Clear the current containers
	m_Folders.clear();
	m_Files.clear();
	if (m_RootFolderNode != nullptr) delete m_RootFolderNode;

	// This method will recursivelly construct the packet tree
	std::function<void(FolderNode*)> CreatePacketTree = [&](FolderNode* _currentNode)
	{
		// For each folder/file recursivelly
		for (auto & p : std::filesystem::directory_iterator(_currentNode->folderPath))
		{
			// Get the path
			auto path = std::filesystem::path(p);

			// Get the relative path
			auto relativePath = path.relative_path().string();

			// Check if this is a file
			if (std::filesystem::is_regular_file(p))
			{
				// Ignore reference extensions and condensed extensions
				if (path.extension().string().compare(ReferenceExtension) != 0 
					&& path.extension().string().compare(CondensedExtension) != 0
					&& path.extension().string().compare(CondensedInfoExtension) != 0)
				{
					// Insert the file path
					_currentNode->files.push_back(relativePath);

					// Insert into the file set
					m_Files.insert(relativePath);
				}
			}
			// Folder
			else
			{
				// Create a new folder node
				FolderNode* folderNode = new FolderNode;
				folderNode->folderPath = relativePath;

				// Insert the folder node
				_currentNode->children.push_back(folderNode);

				// Insert into the folder set
				m_Folders.insert(relativePath);
			}
		}

		// For each child folder
		for (auto& child : _currentNode->children)
		{
			// Recursivelly construct the tree
			CreatePacketTree(child);
		}
	};

	// Create the root folder
	m_RootFolderNode = new FolderNode;
	m_RootFolderNode->folderPath = _packetDirectory;

	// Create the packet tree from the root folder
	CreatePacketTree(m_RootFolderNode);
}

std::vector<std::string> PacketScanner::CheckForHashCollisions()
{
	std::map<HashPrimitive, std::string> fileMaps;
	std::vector<std::string> out;

	// For each file
	for (auto& file : m_Files)
	{
		// Hash the file path
		HashPrimitive fileHash = Hash(file).GetHashValue();

		// Check if we already have this file on our map
		if (fileMaps.find(fileHash) != fileMaps.end())
		{
			// Insert it on our output vector
			out.push_back(file);
		}
		// New key
		else
		{
			// Insert the new key
			fileMaps.insert({ fileHash, file });
		}
	}

	return out;
}

std::vector<std::pair<std::string, std::vector<std::pair<Hash, std::string>>>> PacketScanner::GetFileTreeHashInfos()
{
	std::vector<std::pair<std::string, std::vector<std::pair<Hash, std::string>>>> out;

	// Check if our root folder is valid
	if (m_RootFolderNode == nullptr)
	{
		// Invalid root folder!
		return out;
	}

	std::function<void(FolderNode*)> ProcessUsingTree = [&](FolderNode* _currentNode)
	{
		// Set the new folder pair
		std::pair<std::string, std::vector<std::pair<Hash, std::string>>> folderPair;
		folderPair.first = _currentNode->folderPath;

		// Get a short variable to the file vector
		auto& fileVector = folderPair.second;

		// For each file on this node
		for (auto& file : _currentNode->files)
		{
			// Hash the file path
			Hash fileHash = Hash(file);

			// Insert it on our output vector
			fileVector.push_back({ fileHash, file });
		}

		// Insert the folder pair into the output vector
		out.push_back(folderPair);

		// For each child folder
		for (auto& childFolder : _currentNode->children)
		{
			// Call recursivelly this method
			ProcessUsingTree(childFolder);
		}
	};

	// Call the recursive processing method
	ProcessUsingTree(m_RootFolderNode);

	return out;
}

uint32_t PacketScanner::GetTotalFolderNumber()
{
	return m_Folders.size();
}

uint32_t PacketScanner::GetTotalFileNumber()
{
	return m_Files.size();
}