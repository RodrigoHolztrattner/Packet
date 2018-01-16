////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFragment.h"
#include "PacketStrings.h"

#include <json.hpp>

Packet::PacketFragment::PacketFragment(std::string _fragmentName)
{
	// Set the initial data
	m_FragmentName = _fragmentName;

	// Read the metadata
	bool result = ReadMetadata();
	if (!result)
	{
		// Error!!
		return;
	}

	// Open the data file
	result = OpenDataFile();
	if (!result)
	{
		// Error!!
		return;
	}
}

Packet::PacketFragment::PacketFragment(std::string _fragmentName, uint32_t _maximumSize)
{
	// Set the initial data
	m_FragmentName = _fragmentName;
	m_FragmentMaximumSize = _maximumSize;
	m_FragmentInternalIndexCounter = 0;

	// Create the initial huge section
	SectionMetadata initialSection;
	initialSection.sectionStartingByte = 0;
	initialSection.sectionSize = _maximumSize;
	initialSection.fileIdentifier = -1;

	// Insert the new section
	m_FragmentUnusedSections.push_back(initialSection);

	// Save the initial metadata
	bool result = SaveMetadata();
	if (!result)
	{
		// Error!!
		return;
	}

	// Create the data file
	result = OpenDataFile();
	if (!result)
	{
		// Error!!
		return;
	}
}

Packet::PacketFragment::~PacketFragment()
{
	// Save the metadata
	bool result = SaveMetadata();
	if (!result)
	{
		// Error saving the metadata!
	}

	// Check if the stream file is open
	if (m_FragmentDataStream.is_open())
	{
		// Close the stream file
		m_FragmentDataStream.close();
	}
}

bool Packet::PacketFragment::InsertData(unsigned char* _data, uint32_t _dataSize, FileIdentifier& _fileIdentifier)
{
	// Try to allocate a new section for the data
	SectionMetadata newSection;
	if (!AllocateSection(_dataSize, newSection))
	{
		return false;
	}

	// Insert the data into the file
	if (!WriteDataToFile(_data, newSection.sectionStartingByte, newSection.sectionSize))
	{
		return false;
	}

	// Set the new file identifier and increment the counter
	_fileIdentifier = m_FragmentInternalIndexCounter;
	newSection.fileIdentifier = _fileIdentifier;
	m_FragmentInternalIndexCounter++;

	// Increment the total files count
	m_FragmentTotalFiles++;

	// Insert the section reference into the file mapping
	m_FragmentFileMapping.insert(std::pair<FileIdentifier, SectionMetadata>(_fileIdentifier, newSection));

	return true;
}

bool Packet::PacketFragment::DeleteData(FileIdentifier _fileIdentifier)
{
	// Try to find the file section metadata
	auto it = m_FragmentFileMapping.find(_fileIdentifier);
	if (it == m_FragmentFileMapping.end())
	{
		return false;
	}

	// Set the section variable
	SectionMetadata section = it->second;
	
	// Remove the file entry
	m_FragmentFileMapping.erase(it);

	// Desallocate the section
	DeallocateSection(section);

	// Decrement the file count
	m_FragmentTotalFiles--;

	return true;
}

uint32_t Packet::PacketFragment::GetDataSize(FileIdentifier _fileIdentifier)
{
	// Try to find the file section metadata
	auto it = m_FragmentFileMapping.find(_fileIdentifier);
	if (it != m_FragmentFileMapping.end())
	{
		return it->second.sectionSize;
	}

	return 0;
}

bool Packet::PacketFragment::GetData(unsigned char* _data, FileIdentifier _fileIdentifier)
{
	// Try to find the file section metadata
	auto it = m_FragmentFileMapping.find(_fileIdentifier);
	if (it == m_FragmentFileMapping.end())
	{
		return false;
	}

	// Set the section variable
	SectionMetadata section = it->second;
	
	// Read the data from the file
	if (!ReadDataFromFile(_data, section.sectionStartingByte, section.sectionSize))
	{
		return false;
	}

	return true;
}

bool Packet::PacketFragment::AllocateSection(uint32_t _size, SectionMetadata& _section)
{
	// Check if we are pure
	if (!IsPure())
	{
		return false;
	}

	// Check if it is possible to allocate this size
	if (_size >= m_FragmentMaximumSize)
	{
		return false;
	}

	// For each section
	int bestSectionIndex = -1;
	uint32_t bestSectionSize = -1;
	for (unsigned int i = 0; i < m_FragmentUnusedSections.size(); i++)
	{
		// Check if the current section can accommodate the data
		if (m_FragmentUnusedSections[i].sectionSize >= _size)
		{
			// Check if the empty space is lower
			if (m_FragmentUnusedSections[i].sectionSize < bestSectionSize)
			{
				// Set the new best section size
				bestSectionSize = m_FragmentUnusedSections[i].sectionSize;

				// Set the best section index
				bestSectionIndex = i;
			}
		}
	}

	// Check if we found a best section
	if (bestSectionIndex == -1)
	{
		return false;
	}

	// Set the section data
	_section.sectionStartingByte = m_FragmentUnusedSections[bestSectionIndex].sectionStartingByte;
	_section.sectionSize = _size;

	// Remove the best section from the unnused list
	m_FragmentUnusedSections.erase(m_FragmentUnusedSections.begin() + bestSectionIndex);

	// Calc the remaining size
	uint32_t remainingSize = bestSectionSize - _size;

	// Check if we should insert a new section for the remaining size
	if (remainingSize > 0)
	{
		// Create a new section for the remaining space
		SectionMetadata newSection;
		newSection.sectionStartingByte = _section.sectionStartingByte + _section.sectionSize;
		newSection.sectionSize = remainingSize;

		// Insert the new section
		m_FragmentUnusedSections.push_back(newSection);
	}

	return true;
}

void Packet::PacketFragment::DeallocateSection(SectionMetadata _section)
{
	// Insert the section into the unnused list
	_section.fileIdentifier = -1;
	m_FragmentUnusedSections.push_back(_section);
}

bool Packet::PacketFragment::IsPure()
{
	if (m_FragmentUnusedSections.size() >= FragmentMaximumUnusedSections)
	{
		return false;
	}

	return true;
}

bool Packet::PacketFragment::Optimize()
{
	return true;
}

std::string Packet::PacketFragment::GetName()
{
	return m_FragmentName;
}

bool Packet::PacketFragment::WriteDataToFile(unsigned char* _data, uint32_t _position, uint32_t _size)
{
	// Check if the stream file is open
	if (!m_FragmentDataStream.is_open())
	{
		return false;
	}
	
	// Seek to the position
	m_FragmentDataStream.seekp(_position, std::ios::beg);

	// Write to the file
	m_FragmentDataStream.write((const char*)_data, _size);

	return true;
}

bool Packet::PacketFragment::ReadDataFromFile(unsigned char* _data, uint32_t _position, uint32_t _size)
{
	// Check if the stream file is open
	if (!m_FragmentDataStream.is_open())
	{
		return false;
	}

	// Seek to the position
	m_FragmentDataStream.seekp(_position, std::ios::beg);

	// Read from the file
	m_FragmentDataStream.read((char*)_data, _size);

	return true;
}

bool Packet::PacketFragment::ReadMetadata()
{
	std::ifstream file;
	file.open(m_FragmentName + PacketStrings::FragmentMetadataExtension);

	// Check if the file is open
	if (!file.is_open())
	{
		return false;
	}

	// Our json variable
	nlohmann::json json;

	// Read the json data and close the file
	file >> json;
	file.close();

	// Read the metadata
	m_FragmentTotalFiles = json["FragmentTotalFiles"].get<uint32_t>();
	m_FragmentMaximumSize = json["FragmentMaximumSize"].get<uint32_t>();
	m_FragmentInternalIndexCounter = json["FragmentInternalIndexCounter"].get<uint32_t>();
	m_FragmentUnusedSections = json["FragmentUnusedSections"].get<std::vector<SectionMetadata>>();
	std::vector<SectionMetadata> tempFileMappingData = json["FragmentFileMapping"].get<std::vector<SectionMetadata>>();

	// Transform the fille mapping vector
	for (auto entry : tempFileMappingData)
	{
		m_FragmentFileMapping.insert(std::pair<FileIdentifier, SectionMetadata>(entry.fileIdentifier, entry));
	}

	return true;
}

bool Packet::PacketFragment::SaveMetadata()
{
	// Our json variable
	nlohmann::json json;

	// Transform the file mapping to a vector
	std::vector<SectionMetadata> tempFileMappingData;
	for (auto it = m_FragmentFileMapping.begin(); it != m_FragmentFileMapping.end(); ++it) 
	{
		tempFileMappingData.push_back(it->second);
	}

	// Save the metadata
	json["FragmentTotalFiles"] = m_FragmentTotalFiles;
	json["FragmentMaximumSize"] = m_FragmentMaximumSize;
	json["FragmentInternalIndexCounter"] = m_FragmentInternalIndexCounter;
	json["FragmentUnusedSections"] = m_FragmentUnusedSections;
	json["FragmentFileMapping"] = tempFileMappingData;

	// Set the output file
	std::ofstream file;
	file.open(m_FragmentName + PacketStrings::FragmentMetadataExtension);
	file << json.dump(4);
	file.close();

	return true;
}

bool Packet::PacketFragment::OpenDataFile()
{
	// Try to open the file
	m_FragmentDataStream.open(m_FragmentName + PacketStrings::FragmentDataExtension, std::ios::in | std::ios::out | std::ios::binary);

	// Check if the file is good
	if (!m_FragmentDataStream.good()) 
	{
		// Try to create a new one
		m_FragmentDataStream.open(m_FragmentName + PacketStrings::FragmentDataExtension, std::ios::out | std::ios::binary);

		// Check if the file is good
		if (!m_FragmentDataStream.good())
		{
			return false;
		}

		// Create a temporary buffer for the new data file
		unsigned char* temporaryBuffer = new unsigned char[m_FragmentMaximumSize];

		// Zero the data
		memset(temporaryBuffer, 0, sizeof(unsigned char) * m_FragmentMaximumSize);

		// Write the temporary buffer data
		m_FragmentDataStream.write((const char*)temporaryBuffer, sizeof(unsigned char) * m_FragmentMaximumSize);

		// Close the file
		m_FragmentDataStream.close();

		// Delete the temporary data
		delete[] temporaryBuffer;

		// Try to open the file with the right mode
		m_FragmentDataStream.open(m_FragmentName + PacketStrings::FragmentDataExtension, std::ios::in | std::ios::out | std::ios::binary);

		// Check if the file is good
		if (!m_FragmentDataStream.good())
		{
			return false;
		}
	}

	return true;
}

void Packet::PacketFragment::CloseDataFile()
{
	// Check if the file is open
	if (m_FragmentDataStream.is_open())
	{
		// Close the file stream
		m_FragmentDataStream.close();
	}
}

//////////
// JSON //
//////////

void Packet::to_json(nlohmann::json& _json, const Packet::PacketFragment::SectionMetadata& _object)
{
	_json = nlohmann::json
	{
		{ "SectionStartingByte", _object.sectionStartingByte },
		{ "SectionSize", _object.sectionSize },
		{ "FileIdentifier", _object.fileIdentifier }
	};
}

void Packet::from_json(const nlohmann::json& _json, Packet::PacketFragment::SectionMetadata& _object)
{
	_object.sectionStartingByte = _json.at("SectionStartingByte").get<uint32_t>();
	_object.sectionSize = _json.at("SectionSize").get<uint32_t>();
	_object.fileIdentifier = _json.at("FileIdentifier").get<uint32_t>();
}