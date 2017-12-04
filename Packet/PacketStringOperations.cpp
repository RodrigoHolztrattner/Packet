////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketStringOperations.h"

const std::string Packet::PacketStringOperations::BackDelimiterType = "..";

Packet::PacketStringOperations::PacketStringOperations()
{
	// Set the initial data
	// ...
}

Packet::PacketStringOperations::~PacketStringOperations()
{
}

std::string Packet::PacketStringOperations::GetDirectoryFromPath(std::string& _path)
{
	std::string result;
	const size_t last_slash_idx = _path.rfind(FolderDelimiterType);
	if (std::string::npos != last_slash_idx)
	{
		result = _path.substr(0, last_slash_idx);
	}

	return result;
}

std::string Packet::PacketStringOperations::GetFilenameFromPath(std::string& _path)
{
	std::string result;

	// Split the given path
	std::vector<std::string> splitPath = SplitPath(_path);

	// Check if we got at last one item
	if (splitPath.size())
	{
		// Set the last string
		result = splitPath.back();
	}

	return result;
}

std::vector<std::string> Packet::PacketStringOperations::SplitPath(std::string& str)
{
	std::vector<std::string> result;
	std::set<char> delims{ FolderDelimiterType };

	// Check if the string is just and empty one
	if (str.compare("") == 0)
	{
		return result;
	}

	char const* pch = str.c_str();
	char const* start = pch;
	for (; *pch; ++pch)
	{
		if (delims.find(*pch) != delims.end())
		{
			if (start != pch)
			{
				std::string str(start, pch);
				result.push_back(str);
			}
			else
			{
				result.push_back("");
			}
			start = pch + 1;
		}
	}
	result.push_back(start);

	// For each string
	for (unsigned int i = 0; i < result.size(); i++)
	{
		// Check if this is an empty string
		if (result[i].compare("") == 0)
		{
			// Remove this string
			result.erase(result.begin() + i);
			i--;
		}
	}

	return result;
}

std::string Packet::PacketStringOperations::ComposeDirectory(std::vector<std::string>& _dir)
{
	// Compose the string dir
	std::string stringDir;
	for (auto& folder : _dir)
	{
		stringDir += folder + FolderDelimiterType;
	}

	return stringDir;
}

std::vector<std::string> Packet::PacketStringOperations::JoinDirectorySeek(std::vector<std::string>& _dir, std::vector<std::string>& _seek)
{
	// Copy the dir
	std::vector<std::string> dir = _dir;

	// For each seek command
	for (unsigned int i = 0; i < _seek.size(); i++)
	{
		// Check if this is a "return" command
		if (_seek[i].compare("..") == 0)
		{
			// If we have at last one string on the dir vector
			if (dir.size() > 0)
			{
				// Remove the last entry from the dir
				dir.erase(dir.begin() + dir.size() - 1);
			}
		}
		else
		{
			// Add the seek command to the dir one
			dir.push_back(_seek[i]);
		}
	}

	return dir;
}

bool Packet::PacketStringOperations::PathIsFile(std::string _path)
{
	// Split the given path
	std::vector<std::string> splitPath = SplitPath(_path);

	// Check if we have at last one string
	if (splitPath.size() == 0)
	{
		return false;
	}

	// Search for the back delimiter
	if (_path.find(BackDelimiterType) != std::string::npos)
	{
		return false;
	}

	// Get a short reference to the last string
	std::string& lastString = splitPath[splitPath.size() - 1];

	// Search for the file delimiter
	if(lastString.find(FileDelimiterType) == std::string::npos)
	{
		return false;
	}
		
	return true;
}

bool Packet::PacketStringOperations::PathIsFolder(std::string& _path, bool _ignoreBackDelimiter)
{
	// Split the given path
	std::vector<std::string> splitPath = SplitPath(_path);

	// Check if we have at last one string
	if (splitPath.size() == 0)
	{
		return false;
	}

	// Search for the back delimiter
	if (_path.find(BackDelimiterType) != std::string::npos && _ignoreBackDelimiter)
	{
		return false;
	}

	// Get a short reference to the last string
	std::string& lastString = splitPath[splitPath.size() - 1];

	// Search for the file delimiter
	if (lastString.find(FileDelimiterType) != std::string::npos && lastString.find(BackDelimiterType) == std::string::npos)
	{
		return false;
	}

	return true;
}