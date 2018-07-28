////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketReferenceManager.h"
#include <filesystem>
#include <fstream>

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
																	\
int attr = GetFileAttributes(wstring);								\
if ((attr & FILE_ATTRIBUTE_HIDDEN) == 0)							\
{																	\
	SetFileAttributes(wstring, attr | FILE_ATTRIBUTE_HIDDEN);		\
}

#else

#define MakeFileHidden(wstring)

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

bool PacketReferenceManager::RegisterFileReference(std::string _thisFile, std::string _referencesThis)
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

	// Get the reference vector
	std::vector<FileReference> referenceVector = InternalGetFileReferences(referenceFilePath.string());

	// For each reference
	bool foundInvalidReference = false;
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
			if (resultReference.fileReferencePath.length() != 0)
			{
				// Ok we found a valid substitute
				reference = resultReference;
			}
			else
			{
				// We found a reference that has a problem and can't be fixed
				foundInvalidReference = true;
			}
		}
	}

	// Check if we should save the file
	if (_allOrNothing && foundInvalidReference)
	{
		return false;
	}

	// Save the file references
	InternalSaveFileReferencesVector(referenceFilePath.string(), referenceVector);

	return true;
}

std::vector<PacketReferenceManager::FileReference> PacketReferenceManager::InternalGetFileReferences(std::string _referencePath)
{
	// Read the json file
	std::ifstream file(_referencePath, std::ios::in);
	nlohmann::json j(file);
	file.close();

	// Get the reference vector
	return j["references"].get<std::vector<FileReference>>();
}

void PacketReferenceManager::InternalSaveFileReferencesVector(std::string _referencePath, std::vector<FileReference>& _referencesVector)
{
	// Insert the new file
	nlohmann::json j;
	j["references"] = _referencesVector;

	// Save the file
	std::ofstream file(_referencePath, std::ios::out);
	file << j;
	file.close();
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

	// For each folder/file recursivelly
	for (auto& p : std::experimental::filesystem::recursive_directory_iterator(m_PacketDirectory))
	{
		// Get the path
		auto filePath = std::experimental::filesystem::path(p);

		// Get the relative path
		auto relativePath = filePath.relative_path().string();

		// Check if this is a file
		if (std::experimental::filesystem::is_regular_file(p))
		{
			// Get the file extension and size
			auto extension = filePath.extension().string();
			auto size = uint64_t(std::experimental::filesystem::file_size(p));

			// Switch the fixer type
			switch (_fixer)
			{
				// Case all
				case ReferenceFixer::MatchAll:
				{
					// Compare
					if (relativePath.compare(_fileReference.fileReferencePath) == 0 
						&& extension.compare(_fileReference.fileExtension) == 0 
						&& size == _fileReference.fileSize)
					{
						return { relativePath, extension, size };
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
						return { relativePath, extension, size };
					}
				}
				// Name and extension
				case ReferenceFixer::AtLeastNameAndExtension:
				{
					// Compare
					if (relativePath.compare(_fileReference.fileReferencePath) == 0
						&& extension.compare(_fileReference.fileExtension) == 0)					
					{
						return { relativePath, extension, size };
					}
					break;
				}
				// Name
				case ReferenceFixer::AtLeastName:
				{
					// Compare
					if (relativePath.compare(_fileReference.fileReferencePath) == 0)
					{
						return { relativePath, extension, size };
					}
					break;
				}
			}
		}
	}

	return dummyResult;
}

void __development__Packet::to_json(nlohmann::json& j, const PacketReferenceManager::FileReference& p)
{
	j = nlohmann::json{ { "path", p.fileReferencePath },{ "ext", p.fileExtension },{ "size", p.fileSize } };
}

void __development__Packet::from_json(const nlohmann::json& j, PacketReferenceManager::FileReference& p)
{
	p.fileReferencePath = j.at("path").get<std::string>();
	p.fileExtension = j.at("ext").get<std::string>();
	p.fileSize = j.at("size").get<uint64_t>();
}