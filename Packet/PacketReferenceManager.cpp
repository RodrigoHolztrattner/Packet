////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketReferenceManager.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iomanip>

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketReferenceManager::PacketReferenceManager()
{
	// Set the initial data
	// ...
}

PacketReferenceManager::~PacketReferenceManager()
{
}

#ifdef WIN32

#include <windows.h>
#define MakeFileHidden(wstring)										\
{																	\
int attr = GetFileAttributes(wstring);								\
if ((attr & FILE_ATTRIBUTE_HIDDEN) == 0)							\
{																	\
	SetFileAttributes(wstring, attr | FILE_ATTRIBUTE_HIDDEN);		\
}																	\
}

#define MakeFileVisible(wstring)									\
{																	\
int attr = GetFileAttributes(wstring);								\
if ((attr & FILE_ATTRIBUTE_HIDDEN) != 0)							\
{																	\
	SetFileAttributes(wstring, attr & ~FILE_ATTRIBUTE_HIDDEN);		\
}																	\
}

#else

#define MakeFileHidden(wstring)
#define MakeFileVisible(wstring)

#endif

bool PacketReferenceManager::Initialize(std::string _packetDirectory)
{
	m_PacketDirectory = _packetDirectory;

	return true;
}

void PacketReferenceManager::ClearFileReferences(std::string _filePath)
{
	// Get the main and reference pathes
	auto mainFilePath = std::experimental::filesystem::path(_filePath);
	auto referenceFilePath = std::experimental::filesystem::path(mainFilePath.string().append(ReferenceExtension));

	// Check if the reference extension file already exists, else create it
	if (!FileReferenceExist(referenceFilePath.string()))
	{
		// The reference file doesn't exist
		return;
	}

	// Get the reference vector
	std::vector<FileReference> referenceVector = InternalGetFileReferences(referenceFilePath.string());

	// Clear the vector
	referenceVector.clear();

	// Save the file references
	InternalSaveFileReferencesVector(referenceFilePath.string(), referenceVector);
}

std::vector<std::string> PacketReferenceManager::GetFileReferences(std::string _filePath)
{
	std::vector<std::string> out;

	// Get the main and reference pathes
	auto mainFilePath = std::experimental::filesystem::path(_filePath);
	auto referenceFilePath = std::experimental::filesystem::path(mainFilePath.string().append(ReferenceExtension));

	// Check if the reference extension file already exists, else create it
	if (!FileReferenceExist(referenceFilePath.string()))
	{
		// The reference file doesn't exist
		return out;;
	}

	// Get the reference vector
	std::vector<FileReference> referenceVector = InternalGetFileReferences(referenceFilePath.string());

	// Get each reference path
	for (auto& reference : referenceVector)
	{
		// Insert the reference path
		out.push_back(reference.fileReferencePath);
	}

	return out;
}

bool PacketReferenceManager::RegisterFileReference(std::string _thisFile, std::string _referencesThis, uint64_t _atLocation)
{
	// Get the main and reference pathes
	auto mainFilePath = std::experimental::filesystem::path(_thisFile);
	auto referenceFilePath = std::experimental::filesystem::path(mainFilePath.string().append(ReferenceExtension));

	// Check if the reference extension file already exists, else create it
	if (!FileReferenceExist(referenceFilePath.string(), true))
	{
		// This shuldn't happen
		return false;
	}

	// Get the reference vector
	std::vector<FileReference> referenceVector = InternalGetFileReferences(referenceFilePath.string());

	// Check if we already have this reference
	for (auto& reference : referenceVector)
	{
		// Compare the file paths
		if (reference.fileReferencePath.compare(_referencesThis) == 0)
		{
			// We already have this reference
			return true;
		}
	}

	{
		// Get a iterator to this file and check if it is valid
		auto p = std::experimental::filesystem::path(_referencesThis);
		if (!std::experimental::filesystem::exists(p) || std::experimental::filesystem::is_directory(p))
		{
			// The file doesn't exist or ir a folder path
			return false;
		}

		// Setup the reference info
		FileReference referenceInfo = {};
		referenceInfo.fileReferencePath = _referencesThis;
		referenceInfo.fileExtension = p.extension().string();
		referenceInfo.fileSize = uint64_t(std::experimental::filesystem::file_size(p));
		referenceInfo.ownerFileReferenceLocation = _atLocation;

		// Insert the reference
		referenceVector.push_back(referenceInfo);
	}
	
	// Save the file references
	InternalSaveFileReferencesVector(referenceFilePath.string(), referenceVector);

	return true;
}

bool PacketReferenceManager::ValidateFileReferences(std::string _filePath, ReferenceFixer _fixer, bool _allOrNothing)
{
	// Get the main and reference pathes
	auto mainFilePath = std::experimental::filesystem::path(_filePath);
	auto referenceFilePath = std::experimental::filesystem::path(mainFilePath.string().append(ReferenceExtension));

	// Check if the reference extension file already exists, else create it
	if (!FileReferenceExist(referenceFilePath.string()))
	{
		// The reference file doesn't exist
		return true;
	}

	// Get the reference vector, also declare the reference fixer vector
	std::vector<FileReference> referenceVector = InternalGetFileReferences(referenceFilePath.string());
	std::vector<FileReference> referenceFixerVector;

	// For each reference
	bool foundInvalidReference = false;
	bool needsUpdate = false;
	for (auto& reference : referenceVector)
	{
		// Get the referenced file path
		auto referenceFilePath = std::experimental::filesystem::path(reference.fileReferencePath);

		// Check if the file exist and it's valid
		if (!std::experimental::filesystem::exists(referenceFilePath))
		{
			// We have a problem, check how we should handle this //
			if (_fixer == ReferenceFixer::None)
			{
				// We won't be fixing this
				return false;
			}

			// Try to find a valid file path using the fixer
			auto resultReference = TryFindMatchingFileForReferenceUsingFixer(reference, _fixer);
			if (resultReference.IsValid())
			{
				// Ok we found a valid substitute
				referenceFixerVector.push_back(resultReference);
				needsUpdate = true;
			}
			else
			{
				// We found a reference that has a problem and can't be fixed, insert a dummy reference
				// into our fixer vector
				referenceFixerVector.push_back(FileReference());
				foundInvalidReference = true;
				needsUpdate = true;
			}
		}
		// Reference exist and it's valid
		else
		{
			// Insert a dummy reference into our fixer vector
			referenceFixerVector.push_back(FileReference());
		}
	}

	// Check if the file needs updated references
	if (!needsUpdate)
	{
		return true;
	}

	// Check if we should save the file
	if (_allOrNothing && foundInvalidReference)
	{
		return false;
	}

	// Try to update the owning file with all updated references
	if (!UpdateOwnerFileWithUpdatedReferences(_filePath, referenceVector, referenceFixerVector))
	{
		return false;
	}

	// Construct the updated reference vector
	std::vector<FileReference> updatedReferences;
	for (int i = 0; i < referenceVector.size(); i++)
	{
		// Check what reference we should use
		if (referenceFixerVector[i].IsValid())
		{
			// Use the fixer one
			updatedReferences.push_back(referenceFixerVector[i]);
		}
		else
		{
			// Use the original one
			updatedReferences.push_back(referenceVector[i]);
		}
	}

	// Save the file references
	InternalSaveFileReferencesVector(referenceFilePath.string(), updatedReferences);

	return true;
}

std::vector<PacketReferenceManager::FileReference> PacketReferenceManager::InternalGetFileReferences(std::string _referencePath)
{
	// Read the json file
	std::ifstream file(_referencePath, std::ios::in);
	nlohmann::json j;
	file >> j;
	file.close();

	// Get the reference vector
	return j["references"].get<std::vector<FileReference>>();
}

void PacketReferenceManager::InternalSaveFileReferencesVector(std::string _referencePath, std::vector<FileReference>& _referencesVector)
{
	// Insert the new file
	nlohmann::json j;
	j["references"] = _referencesVector;

	// Make the reference info file visible again (if on windows)
	MakeFileVisible(std::experimental::filesystem::path(_referencePath).wstring().c_str());

	// Save the file
	std::ofstream file(_referencePath, std::ios::out);
	file << std::setw(4) << j << std::endl;
	file.close();

	// Hide the reference info file (if on windows)
	MakeFileHidden(std::experimental::filesystem::path(_referencePath).wstring().c_str());
}

bool PacketReferenceManager::FileReferenceExist(std::string _referencePath, bool _create)
{
	// Check if the reference extension file already exists
	auto p = std::experimental::filesystem::path(_referencePath);
	if (!std::experimental::filesystem::exists(p))
	{
		// If we shouldn't create the file
		if (!_create)
		{
			return false;
		}

		// Create the extension file
		std::vector<FileReference> dummyReferenceArray;
		nlohmann::json j;
		j["references"] = dummyReferenceArray;
		std::ofstream file(_referencePath, std::ios::out);
		file << j;
		file.close();

		// Make the file hidden (if we are on windows)
		MakeFileHidden(std::experimental::filesystem::path(_referencePath).wstring().c_str());
	}

	return true;
}

PacketReferenceManager::FileReference PacketReferenceManager::TryFindMatchingFileForReferenceUsingFixer(FileReference& _fileReference, ReferenceFixer _fixer)
{
	FileReference dummyResult;

	// Check if we are using a fixer
	if (_fixer == ReferenceFixer::None)
	{
		return dummyResult;
	}

	// Get the reference filename
	auto referenceFilename = std::experimental::filesystem::path(_fileReference.fileReferencePath).filename().string();

	// For each folder/file recursivelly
	for (auto& p : std::experimental::filesystem::recursive_directory_iterator(m_PacketDirectory))
	{
		// Get the path
		auto filePath = std::experimental::filesystem::path(p);

		// Get the relative path
		auto relativePath = filePath.relative_path().string();

		// Get the file name
		auto fileName = filePath.filename().string();

		// Check if this is a file
		if (std::experimental::filesystem::is_regular_file(p))
		{
			// Get the file properties
			auto extension = filePath.extension().string();
			auto size = uint64_t(std::experimental::filesystem::file_size(p));
			auto location = _fileReference.ownerFileReferenceLocation;

			// Switch the fixer type
			switch (_fixer)
			{
				// Case all
				case ReferenceFixer::MatchAll:
				{
					// Compare
					if (fileName.compare(referenceFilename) == 0
						&& extension.compare(_fileReference.fileExtension) == 0 
						&& size == _fileReference.fileSize)
					{
						return { relativePath, extension, size, location };
					}
					break;
				}
				// (Name and extension) or (size and extension)
				case ReferenceFixer::NameAndExtensionOrSizeAndExtension:
				{
					// Compare
					if (extension.compare(_fileReference.fileExtension) == 0
						&& size == _fileReference.fileSize)
					{
						return { relativePath, extension, size, location };
					}
				}
				// Name and extension
				case ReferenceFixer::AtLeastNameAndExtension:
				{
					// Compare
					if (fileName.compare(referenceFilename) == 0
						&& extension.compare(_fileReference.fileExtension) == 0)					
					{
						return { relativePath, extension, size, location };
					}
					break;
				}
				// Name
				case ReferenceFixer::AtLeastName:
				{
					// Compare
					if (fileName.compare(referenceFilename) == 0)
					{
						return { relativePath, extension, size, location };
					}
					break;
				}
			}
		}
	}

	return dummyResult;
}

bool PacketReferenceManager::UpdateOwnerFileWithUpdatedReferences(std::string _filePath, std::vector<FileReference>& _oldReferences, std::vector<FileReference>& _newReferences)
{
	// Both vectors need to be the same size
	if (_oldReferences.size() != _newReferences.size())
	{
		// Invalid vectors
		return false;
	}

	// Open the owning file
	std::ifstream readFile(_filePath, std::ios::binary);
	if (!readFile.is_open())
	{
		// Error openning the file!
		std::cout << "Error trying to open a file to update its references, the file path is: " << _filePath << std::endl;

		return false;
	}

	// First we need to verify if the owning file has its old references on the same locations they are located here //

	// For each old reference, check if the strings are equal
	for (auto& reference : _oldReferences)
	{
		// The path string
		Path path = {};

		// Set the file position inside the owning file
		readFile.seekg(reference.ownerFileReferenceLocation, std::ios_base::beg);

		// Read the path
		readFile.read((char*)&path, sizeof(Path));

		// Check if both strings are equal
		if (!path.Compare(reference.fileReferencePath.c_str()))
		{
			// Error verifying this reference
			std::cout << "Error verifying a file reference, the file path is: \"" << _filePath << "\" and the location is at: " << reference.ownerFileReferenceLocation << std::endl;

			return false;
		}
	}

	// Close the file
	readFile.close();

	// If we are here the owning file has its old references on the correct location and we are ready to update them //

	// Open the owning file in read mode
	std::ofstream writeFile(_filePath, std::ios::binary);
	if (!writeFile.is_open())
	{
		// Error openning the file!
		std::cout << "Error trying to open a file to update its references, the file path is: " << _filePath << std::endl;

		return false;
	}

	// For each reference (dont matter what vector size we use here)
	for (unsigned int i=0; i<_newReferences.size(); i++)
	{
		// Get the new reference
		auto& newReference = _newReferences[i];

		// Check if this reference needs to be updated
		if (!newReference.IsValid())
		{
			// Just ignore it
			continue;
		}

		// The path string
		Path path = newReference.fileReferencePath;

		// Set the file position inside the owning file
		writeFile.seekp(newReference.ownerFileReferenceLocation, std::ios_base::beg);

		// Write the reference
		writeFile.write((char*)&path, sizeof(Path));
	}

	// Close the file
	writeFile.close();

	return true;
}

void __development__Packet::to_json(nlohmann::json& j, const PacketReferenceManager::FileReference& p)
{
	j = nlohmann::json{ { "path", p.fileReferencePath },{ "ext", p.fileExtension },{ "size", p.fileSize },{ "location", p.ownerFileReferenceLocation } };
}

void __development__Packet::from_json(const nlohmann::json& j, PacketReferenceManager::FileReference& p)
{
	p.fileReferencePath = j.at("path").get<std::string>();
	p.fileExtension = j.at("ext").get<std::string>();
	p.fileSize = j.at("size").get<uint64_t>();
	p.ownerFileReferenceLocation = j.at("location").get<uint64_t>();
}