////////////////////////////////////////////////////////////////////////////////
// Filename: PacketObjectManager.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketFragment.h"

#include <string>
#include <vector>

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
PacketNamespaceBegin(Packet)

////////////////
// FORWARDING //
////////////////

////////////////
// STRUCTURES //
////////////////

/*
	- Responsável por gerenciar diretamente os arquivos.
	- Faz uso dos fragments para gravação.
	- Deve prover funcionalidades de inserção e retirada de arquivos, assim como localização dos mesmos por uma hash.
	- Deve permitir otimizar localmente os espaços dos fragments.
	- Deve permitir fazer um agrupamento de itens selecionados (para que os mesmos se encontrem proximos localmente e de rápida leitura).
*/

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketObjectManager
////////////////////////////////////////////////////////////////////////////////
class PacketObjectManager
{
private:

	// The fragment complement name
	const std::string FragmentComplementName = "frag";

	// The fragment info
	struct FragmentInfo
	{
		// The fragment name
		std::string fragmentName;
	};

public:

	// The file fragment identifier type
	struct FileFragmentIdentifier
	{
		// The fragment that have the file
		std::string fragmentName;

		// The fragment index
		uint32_t fragmentIndex;

		// The file identifier inside the fragment
		PacketFragment::FileIdentifier fileIdentifier;
	};

public:

	// The packet attributes
	struct PacketAttributes
	{
		// The packet object name
		std::string packetObjectName;

		// The maximum fragment size (this cannot be changed after set and has a maximum limit of 4.294.967.295 bytes or 4.2gb)
		uint32_t maximumFragmentSize;
	};

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketObjectManager();
	~PacketObjectManager();

//////////////////
// MAIN METHODS //
public: //////////

	// Insert a file into the packet object
	bool InsertFile(std::string _filePathOrigin, FileFragmentIdentifier& _hashidentifier);

	// Insert a chunk of data
	bool InsertData(unsigned char* _data, uint32_t _size, FileFragmentIdentifier& _hashidentifier);

	// Remove a file from this packet object
	bool RemoveFile(FileFragmentIdentifier _hashIdentifier);

	// Criar uma função que recebe uma função lambda de parâmetro que usaremos quando na otmização um hash identifier trocar de estado
	// ... TODO

	// Set the packet atributes (REMOVE THIS)
	void SetPacketAttributes(PacketAttributes _attributes)
	{
		m_PacketObjectAttributes = _attributes;
	}

private:

	// Return a valid fragment object (creating one if necessary)
	PacketFragment* GetValidFragment();

	// Create a new fragment object
	PacketFragment* CreateNewFragment();

///////////////
// VARIABLES //
private: //////

	// The packet object attributes
	PacketAttributes m_PacketObjectAttributes;

	// All the fragment infos
	std::vector<FragmentInfo> m_FragmentInfos;

	// All fragment objects
	std::vector<PacketFragment*> m_Fragments;
};

// Packet data explorer
PacketNamespaceEnd(Packet)
