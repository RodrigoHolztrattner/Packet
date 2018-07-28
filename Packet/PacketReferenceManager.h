////////////////////////////////////////////////////////////////////////////////
// Filename: PacketReferenceManager.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"

#include <string>
#include <vector>
#include <nlohmann\json.hpp>

///////////////
// NAMESPACE //
///////////////

/////////////
// DEFINES //
/////////////

////////////
// GLOBAL //
////////////

///////////////
// NAMESPACE //
///////////////

// Packet data explorer
PacketDevelopmentNamespaceBegin(Packet)

////////////////
// FORWARDING //
////////////////

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketReferenceManager
////////////////////////////////////////////////////////////////////////////////
class PacketReferenceManager
{
public:

	// The reference fixer types
	enum class ReferenceFixer
	{
		None, 
		MatchAll, 
		AtLeastNameAndExtension, 
		AtLeastName, 
		NameAndExtensionOrSizeAndExtension
	};

	// The reference type
	struct FileReference
	{
		// The file properties
		std::string fileReferencePath;

		// The file extension
		std::string fileExtension;

		// The file size
		uint64_t fileSize = 0;
	};

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketReferenceManager();
	~PacketReferenceManager();

//////////////////
// MAIN METHODS //
public: //////////

	// Initialize this packet reference manager
	bool Initialize(std::string _packetDirectory);

	// This method will register that a given file references another, creating a link between both
	bool RegisterFileReference(std::string _thisFile, std::string _referencesThis);

	// Clear all references for the given file
	void ClearFileReferences(std::string _filePath);

	// This method will validade the given file references, checking if they exist and optionally 
	// will try to fix if there are missing files, we can set to only fix the file references if 
	// all of them are valid or can be fixed
	bool ValidateFileReferences(std::string _filePath, ReferenceFixer _fixer = ReferenceFixer::None, bool _allOrNothing = true);

	// This method will return all references that a given file has
	std::vector<std::string> GetFileReferences(std::string _filePath);

private:

	// Check if a reference file exist, opitionally create it
	bool FileReferenceExist(std::string _referencePath, bool _create = false);

	// This method will return all file references
	std::vector<FileReference> InternalGetFileReferences(std::string _referencePath);

	// This method will save the file reference vector
	void InternalSaveFileReferencesVector(std::string _referencePath, std::vector<FileReference>& _referencesVector);

	// This method will try to find a file inside the packet directories that match the given reference 
	// using a fixer
	FileReference TryFindMatchingFileForReferenceUsingFixer(FileReference& _fileReference, ReferenceFixer _fixer);

///////////////
// VARIABLES //
private: //////

	// The packet reference directory
	std::string m_PacketDirectory;
};

void to_json(nlohmann::json& j, const PacketReferenceManager::FileReference& p);
void from_json(const nlohmann::json& j, PacketReferenceManager::FileReference& p);

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
