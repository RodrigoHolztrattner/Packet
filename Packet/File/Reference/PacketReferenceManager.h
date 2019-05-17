////////////////////////////////////////////////////////////////////////////////
// Filename: PacketReferenceManager.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "../../PacketConfig.h"
#include <string>
#include <vector>
#include <set>

///////////////
// NAMESPACE //
///////////////

// Packet data explorer
PacketDevelopmentNamespaceBegin(Packet)

// Classes we know
class PacketFile;
class PacketFileLoader;
class PacketFileSaver;

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketReferenceManager
////////////////////////////////////////////////////////////////////////////////
class PacketReferenceManager
{
public:


//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketReferenceManager();
	~PacketReferenceManager();

    // Set auxiliary object pointers
    void SetAuxiliarObjects(const PacketFileLoader* _file_loader, const PacketFileSaver* _file_saver);

    // This method will receive a set of dependencies and a file path, it will register a link for each
    // file inside that map targeting the given file path
    bool RegisterDependenciesForFile(std::set<Path> _file_dependencies, Path _file_path) const;

    // Add a reference link to a given file
    bool AddReferenceLink(Path _file_path, Path _reference) const;

    // Remove a reference link from a given file
    bool RemoveReferenceLink(Path _file_path, Path _reference) const;

    // Update a file dependencies, modifying these dependencies to reference a different path
    bool SubstituteDependencyReferences(std::set<Path> _referenced_files_paths, Path _old_path, Path _new_path) const;

    // Make all linked files point to another reference path, so if the file pointer to the 
    // _old_path variable changed its path to _new_path, it will alert all files that it
    // depends on to update their links, making them reference this new path
    bool RedirectLinksFromDependencies(std::set<Path> _referenced_files_paths, Path _old_path, Path _new_path) const;

    // This method will load the original file, retrieve its dependencies and get the difference
    // between the current file, returning 2 maps, the first is the dependencies that must be
    // removed and the second is all dependencies that must be added
    std::pair<std::set<Path>, std::set<Path>> RetrieveDependencyDiffFromOriginalFile(const std::unique_ptr<PacketFile>& _file) const;

//////////////////
// MAIN METHODS //
public: //////////

    // Find all instances of the input path on the given file raw data and substitute them for the other input one
    void SubstituteAllPathReferences(std::vector<uint8_t>& _file_data, Path _lookup_path, Path _new_path) const;

///////////////
// VARIABLES //
private: //////

    // Our file loader and importer references
    const PacketFileLoader*   m_FileLoaderPtr;
    const PacketFileSaver*    m_FileSaverPtr;

};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
