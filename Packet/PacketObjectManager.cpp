////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketObjectManager.h"
#include "PacketFileDataOperations.h"
#include "PacketStrings.h"

Packet::PacketObjectManager::PacketObjectManager()
{

}

Packet::PacketObjectManager::~PacketObjectManager()
{
}

bool Packet::PacketObjectManager::InitializeEmpty(std::string _packetName, uint32_t _maximumFragmentSize)
{
	// Set the initial data
	m_PacketObjectAttributes = PacketAttributes(_packetName, _maximumFragmentSize);

	return true;
}

bool Packet::PacketObjectManager::InitializeFromData(std::vector<unsigned char>& _data, uint32_t& _location, std::string _packetName, uint32_t _maximumFragmentSize)
{
	// Set the initial data
	m_PacketObjectAttributes = PacketAttributes(_packetName, _maximumFragmentSize);

	// Read the number of fragment infos
	uint32_t numberFragmentInfos = PacketFileDataOperations::ReadFromData<uint32_t>(_data, _location);

	// For each fragment info
	for (unsigned int i = 0; i < numberFragmentInfos; i++)
	{
		// The new fragment info
		FragmentInfo fragmentInfo;

		// Save the fragment name
		fragmentInfo.fragmentName = PacketFileDataOperations::ReadFromData(_data, _location);

		// Inser the fragment info into our vector
		m_FragmentInfos.push_back(fragmentInfo);

		// Create the new packet fragment object
		PacketFragment* newFragmentObject = new PacketFragment(fragmentInfo.fragmentName);

		// Insert it into our list of fragment objects
		m_Fragments.push_back(newFragmentObject);
	}

	return true;
}

std::vector<unsigned char> Packet::PacketObjectManager::Serialize()
{
	// The data
	std::vector<unsigned char> data;
	uint32_t location = 0;

	// Write the number of fragment infos
	PacketFileDataOperations::SaveToData<uint32_t>(data, location, m_FragmentInfos.size());

	// For each fragment info
	for (auto & fragmentInfo : m_FragmentInfos)
	{
		// Save the fragment name
		PacketFileDataOperations::SaveToData(data, location, fragmentInfo.fragmentName);
	}

	// For each fragment
	for (auto & fragment : m_Fragments)
	{
		// Save this fragment metadata (the data is saved automatically)
		fragment->SaveMetadata();
	}

	return data;
}

bool Packet::PacketObjectManager::InsertFile(std::string _filePathOrigin, FileFragmentIdentifier& _hashidentifier)
{
	// Open the file and get the size
	std::ifstream file(_filePathOrigin, std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	// Check if the size is valid
	if (size == -1)
	{
		return false;
	}

	// Read the data
	std::vector<unsigned char> buffer((unsigned int)size);
	if (file.read((char*)buffer.data(), size))
	{
		// Close the file
		file.close();

		// Insert the data
		return InsertData(buffer.data(), buffer.size(), _hashidentifier);
	}

	// Close the file
	file.close();

	return false;
}

bool Packet::PacketObjectManager::InsertData(unsigned char* _data, uint32_t _size, FileFragmentIdentifier& _hashidentifier)
{
	// Check if the data and the size are valid
	if (_data == nullptr || _size == 0)
	{
		return false;
	}

	// Check if the size is bigger then our maximum allowed size and we cant handle it
	if (_size >= m_PacketObjectAttributes.maximumFragmentSize)
	{
		return false;
	}

	// Get a valid fragment object
	// PacketFragment* fragment = GetValidFragment();
	PacketFragment* fragment = GetValidFragmentForSize(_size);

	// Try to insert the data into this fragment
	PacketFragment::FileIdentifier fileIdentifier;
	if (!fragment->InsertData(_data, _size, fileIdentifier))
	{
		// Create a new fragment
		fragment = CreateNewFragment();

		// Try to insert the data now into the new fragment
		if (!fragment->InsertData(_data, _size, fileIdentifier))
		{
			return false;
		}
	}

	// Set the file hash identifier
	// _hashidentifier.fragmentName = fragment->GetName();
	_hashidentifier.fileIdentifier = fileIdentifier;
	_hashidentifier.fragmentIndex = m_Fragments.size() - 1;

	return true;
}

uint32_t Packet::PacketObjectManager::GetFileSize(FileFragmentIdentifier _hashidentifier)
{
	// Get the file fragment
	PacketFragment* fragment = GetFragmentWithIndex(_hashidentifier.fragmentIndex);
	if (fragment == nullptr)
	{
		return 0;
	}

	return fragment->GetDataSize(_hashidentifier.fileIdentifier);
}

bool Packet::PacketObjectManager::GetFile(std::string _filePathDestination, FileFragmentIdentifier _hashidentifier)
{
	// Get the data size and check if it is valid
	uint32_t fileSize = GetFileSize(_hashidentifier);
	if (fileSize == 0)
	{
		return false;
	}

	// Allocate the temporary data
	unsigned char* temporaryData = new unsigned char[fileSize];

	// Get the data and check if everything is correct
	if (!GetData(temporaryData, _hashidentifier))
	{
		// Desalloc the temporary data
		delete[] temporaryData;

		return false;
	}

	// Create the file
	std::ofstream file(_filePathDestination, std::ios::binary);

	// Check if the file was created successfully
	if (!file.is_open())
	{
		// Desalloc the temporary data
		delete[] temporaryData;

		return false;
	}

	// Write the data into the file
	file.write((char*)temporaryData, sizeof(unsigned char) * fileSize);
	
	// Close the file
	file.close();

	// Desalloc the temporary data
	delete[] temporaryData;

	return true;
}

bool Packet::PacketObjectManager::GetData(unsigned char* _data, FileFragmentIdentifier _hashidentifier)
{
	// Check if the data is valid
	if (_data == nullptr)
	{
		return false;
	}

	// Get the file fragment
	PacketFragment* fragment = GetFragmentWithIndex(_hashidentifier.fragmentIndex);
	if (fragment == nullptr)
	{
		return false;
	}

	// Return the data
	return fragment->GetData(_data, _hashidentifier.fileIdentifier);
}

bool Packet::PacketObjectManager::RemoveFile(FileFragmentIdentifier _hashIdentifier)
{
	// Check if the hash is valid
	if (_hashIdentifier.fragmentIndex < 0 || _hashIdentifier.fragmentIndex >= m_Fragments.size())
	{
		return false;
	}

	// Get the fragment object
	PacketFragment* fragment = m_Fragments[_hashIdentifier.fragmentIndex];

	// Remove the file
	bool result = fragment->DeleteData(_hashIdentifier.fileIdentifier);
	if (!result)
	{
		return false;
	}

	return true;
}

Packet::PacketFragment* Packet::PacketObjectManager::GetValidFragment()
{
	// Check if we have at last one fragment
	if (m_Fragments.size() == 0)
	{
		// Create a new fragment
		return CreateNewFragment();
	}

	return m_Fragments[m_Fragments.size() - 1];
}

Packet::PacketFragment* Packet::PacketObjectManager::GetValidFragmentForSize(uint32_t _size)
{
	// For each fragment
	for (auto& fragment : m_Fragments)
	{
		// Check if this fragment has an unused section with at last the input size
		if (fragment->HasUnusedSectionWithAtLast(_size))
		{
			return fragment;
		}
	}

	// Create a new fragment
	return CreateNewFragment();
}

Packet::PacketFragment* Packet::PacketObjectManager::GetFragmentWithIndex(uint32_t _index)
{
	// Check the index
	if (_index < 0 || _index >= m_Fragments.size())
	{
		return nullptr;
	}

	return m_Fragments[_index];
}

Packet::PacketFragment* Packet::PacketObjectManager::CreateNewFragment()
{
	// Get the total number of fragments
	uint32_t totalFragments = m_Fragments.size();

	// Compose the new fragment name
	std::string fragmentName = m_PacketObjectAttributes.packetObjectName + PacketStrings::FragmentComplementName + std::to_string(totalFragments);

	// Create the new fragment info
	FragmentInfo newFragmentInfo;
	newFragmentInfo.fragmentName = fragmentName;

	// Create the new fragment object
	PacketFragment* newFragment = new PacketFragment(fragmentName, m_PacketObjectAttributes.maximumFragmentSize);

	// Insert the fragment info
	m_FragmentInfos.push_back(newFragmentInfo);

	// Insert the new fragment object
	m_Fragments.push_back(newFragment);

	return newFragment;
}
