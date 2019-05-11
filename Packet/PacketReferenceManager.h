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
#include <set>

///////////////
// NAMESPACE //
///////////////

// Packet data explorer
PacketDevelopmentNamespaceBegin(Packet)

// Classes we know
class PacketFile;
class PacketFileLoader;
class PacketFileImporter;

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
	PacketReferenceManager(const PacketFileLoader& _file_loader, const PacketFileImporter& _file_importer);
	~PacketReferenceManager();

    // This method will receive a set of dependencies and a file path, it will register a link for each
    // file inside that map targeting the given file path
    bool RegisterDependenciesForFile(std::set<Path> _file_dependencies, Path _file_path) const;

    // Add a dependency for the given file path
    bool AddDependency(Path _file_path, Path _dependency_path) const;

    // Remove a dependency for the given file path
    bool RemoveDependency(Path _file_path, Path _dependency_path) const;

    // Add a reference link to a given file
    bool AddReferenceLink(Path _file_path, Path _reference) const;

    // Remove a reference link from a given file
    bool RemoveReferenceLink(Path _file_path, Path _reference) const;

    // Make all referenced files point to another reference path
    bool RedirectLinks(std::set<Path> _referenced_files_paths, Path _old_path, Path _new_path) const;

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
    const PacketFileLoader& m_FileLoader;
    const PacketFileImporter& m_FileImporter;

};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
