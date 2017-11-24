////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketStringOperations.h"

Packet::PacketStringOperations::PacketStringOperations()
{
	// Set the initial data
	// ...
}

Packet::PacketStringOperations::~PacketStringOperations()
{
}

std::string Packet::PacketStringOperations::GetDirectoryFrompath(std::string _path)
{
	std::string result;
	const size_t last_slash_idx = _path.rfind(DelimiterType);
	if (std::string::npos != last_slash_idx)
	{
		result = _path.substr(0, last_slash_idx);
	}

	return result;
}

std::string Packet::PacketStringOperations::GetFilenameFromPath(std::string _path)
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
	std::set<char> delims{ DelimiterType };

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

	return result;
}