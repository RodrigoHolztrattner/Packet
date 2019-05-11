////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileReferences.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "../PacketConfig.h"
#include <string>
#include <unordered_map>
#include <set>

///////////////
// NAMESPACE //
///////////////

// Packet data explorer
PacketDevelopmentNamespaceBegin(Packet)

// Classes we know
class PacketFile;
class PacketReferenceManager;

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFileReferences
////////////////////////////////////////////////////////////////////////////////
class PacketFileReferences
{
    // Friend classes
    friend PacketFile;
    friend PacketReferenceManager;

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFileReferences();
	~PacketFileReferences();

//////////////////
// MAIN METHODS //
public: //////////

    // Return a set of all files that depends on this one
    const std::set<Path>& GetFileLinks() const;

    // Return a set of all files that this one depends on
    const std::set<Path>& GetFileDependencies() const;

    static PacketFileReferences CreateFromData(const std::vector<uint8_t>& _data);

    static PacketFileReferences CreateFromJSON(nlohmann::json _json);

protected:

    static PacketFileReferences CreateFromSets(std::set<Path> _file_links,
                                               std::set<Path> _file_dependencies);

    nlohmann::json SaveIntoJSON() const;

    // Add a new entry on the files that depends on this one
    void AddFileLink(Path _new_link);

    // Remove an entry on the files that depends on this one
    void RemoveFileLink(Path _link);
 
///////////////
// VARIABLES //
protected: ////

    std::set<Path> m_FileLinks;
    std::set<Path> m_FileDependencies;
};

// Packet data explorer
PacketDevelopmentNamespaceEnd(Packet)
