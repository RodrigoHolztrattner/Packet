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
	uint32_t fragmentIndex;
	PacketFragment* fragment = GetValidFragmentForSize(_size, fragmentIndex); // PacketFragment* fragment = GetValidFragment();

	// Try to insert the data into this fragment
	PacketFragment::FileIdentifier fileIdentifier;
	if (!fragment->InsertData(_data, _size, fileIdentifier))
	{
		return false;
	}

	// Set the file hash identifier
	// _hashidentifier.fragmentName = fragment->GetName();
	_hashidentifier.fileIdentifier = fileIdentifier;
	_hashidentifier.fragmentIndex = fragmentIndex;

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

Packet::PacketFragment* Packet::PacketObjectManager::GetValidFragment(uint32_t& _fragmentIndex)
{
	// Check if we have at last one fragment
	if (m_Fragments.size() == 0)
	{
		// Create a new fragment
		_fragmentIndex = 0;
		return CreateNewFragment();
	}

	_fragmentIndex = m_Fragments.size() - 1;
	return m_Fragments[m_Fragments.size() - 1];
}

Packet::PacketFragment* Packet::PacketObjectManager::GetValidFragmentForSize(uint32_t _size, uint32_t& _fragmentIndex)
{
	// For each fragment
	for (int i=0; i<m_Fragments.size(); i++)
	{
		// Get the current fragment
		auto& fragment = m_Fragments[i];

		// Check if this fragment has an unused section with at last the input size
		if (fragment->HasUnusedSectionWithAtLast(_size))
		{
			_fragmentIndex = i;
			return fragment;
		}
	}

	// Create a new fragment
	_fragmentIndex = m_Fragments.size();
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

/*
	- Cada fragmento deve ter uma funcao de finalizacao para ser otimizado, onde ele renomeia o arquivo aberto para 
	um nome de backup.
	- O ObjectManager deve pegar todos os fragmentos antigos e coloca-los em um vector temporario de backup, zerando o 
	vetor original.
	- Devemos calcular a melhor forma de insercao, dividindo os arquivos em buckets (do tamanho maximo possivel para um 
	fragmento).
	- Apos determinar a ordem, realizar a insercao dos arquivos antigos (lista de backup) normalmente como se fosse um 
	dado novo, atualizando na hash o identificador.

*/
bool Packet::PacketObjectManager::Otimize(std::vector<FileFragmentIdentifier> _allFileFragmentIdentifiers, std::vector<FileFragmentIdentifier>& _outputFileFragmentIdentifiers)
{
	// Save the old fragments and their infos
	std::vector<FragmentInfo> oldFragmentInfos = m_FragmentInfos;
	std::vector<PacketFragment*> oldFragments = m_Fragments;

	// Clear the fragment data
	m_FragmentInfos.clear();
	m_Fragments.clear();

	// For each old fragment
	for (auto& fragment : oldFragments)
	{
		// Rename this fragment (to a temporary name)
		fragment->Rename(fragment->GetName(), PacketStrings::FragmentTemporaryComplementName);
	}

	// For each file fragment identifier
	std::map<int, FileFragmentIdentifier, std::greater<int>> insertionMap;
	for (auto& fileFragmentIdentifier : _allFileFragmentIdentifiers)
	{
		// Check the fragment index
		if (fileFragmentIdentifier.fragmentIndex < 0 || fileFragmentIdentifier.fragmentIndex >= oldFragments.size())
		{
			// Invalid fragment index
			return false;
		}

		// Get the fragment object
		PacketFragment* fragment = oldFragments[fileFragmentIdentifier.fragmentIndex];
		
		// Get the file size
		uint32_t fileSize = fragment->GetDataSize(fileFragmentIdentifier.fileIdentifier);

		// Insert the identifier into our ordering map
		insertionMap.insert(std::pair<int, FileFragmentIdentifier>(fileSize, fileFragmentIdentifier));
	}

	// Allocate enough data that we will need for moving the files
	unsigned char* temporaryData = new unsigned char[m_PacketObjectAttributes.maximumFragmentSize * sizeof(unsigned char)];

	// For each file identifier inside our insertion map
	for (auto& mapEntry : insertionMap)
	{
		// Get the file fragment identifier
		FileFragmentIdentifier& fileFragmentIdentifier = mapEntry.second;

		// Our fragment index should be valid here (we already checked it above) so just get the fragment object
		PacketFragment* fragment = oldFragments[fileFragmentIdentifier.fragmentIndex];

		// Get the file data
		fragment->GetData(temporaryData, mapEntry.first);

		// Insert the data into our new fragments
		FileFragmentIdentifier newFileFragmentIdentifier;
		bool result = InsertData(temporaryData, mapEntry.first, newFileFragmentIdentifier);
		if (!result)
		{
			// Problem inserting the data
			return false;
		}

		// Insert the new file fragment identifier into the output vector
		_outputFileFragmentIdentifiers.push_back(newFileFragmentIdentifier);
	}

	// Delete the temporary data
	delete[] temporaryData;

	// Delete each old fragment
	for (auto& fragment : oldFragments)
	{
		// Shutdown the fragment
		// ... TODO

		// Delete the fragment
		delete fragment;
	}

	return true;
}