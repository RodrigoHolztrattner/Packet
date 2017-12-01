////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketObjectManager.h"

Packet::PacketObjectManager::PacketObjectManager(PacketObjectManager::PacketAttributes _packetAttributes)
{
	// Set the initial data
	m_PacketObjectAttributes = _packetAttributes;
}

Packet::PacketObjectManager::~PacketObjectManager()
{
}

bool Packet::PacketObjectManager::InsertFile(std::string _filePathOrigin, FileFragmentIdentifier& _hashidentifier)
{
	// Open the file and get the size
	std::ifstream file(_filePathOrigin, std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

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

	// Get a valid fragment object
	PacketFragment* fragment = GetValidFragment();

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
	_hashidentifier.fragmentName = fragment->GetName();
	_hashidentifier.fileIdentifier = fileIdentifier;
	_hashidentifier.fragmentIndex = m_Fragments.size() - 1;

	return true;
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

Packet::PacketFragment* Packet::PacketObjectManager::CreateNewFragment()
{
	// Get the total number of fragments
	uint32_t totalFragments = m_Fragments.size();

	// Compose the new fragment name
	std::string fragmentName = m_PacketObjectAttributes.packetObjectName + FragmentComplementName + std::to_string(totalFragments);

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
