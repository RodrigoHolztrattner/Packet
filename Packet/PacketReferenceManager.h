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

    // Add a reference link to a given file
    bool AddReferenceLink(Path _file_path, Path _reference) const;

    // Remove a reference link from a given file
    bool RemoveReferenceLink(Path _file_path, Path _reference) const;

    // Make all referenced files point to another reference path
    bool RedirectReferences(std::set<Path> _referenced_files_paths, Path _old_path, Path _new_path) const;

//////////////////
// MAIN METHODS //
public: //////////

    // Find all instances of the input path on the given file raw data and substitute them for the other input one
    void SubstituteAllPathReferences(std::vector<uint8_t>& _file_data, Path _lookup_path, Path _new_path) const;

///////////////
// VARIABLES //
private: //////

    // Our file loader and importer references
    const PacketFileLoader& m_FileLoaderReference;
    const PacketFileImporter& m_FileImporterReference;

};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
