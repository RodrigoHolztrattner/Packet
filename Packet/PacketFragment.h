////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFragment.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"

#include <json.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <map>

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
	? Tenho que colocar um espaço aqui que são os dados, só que ele deve ser dividido em sessões (declarar essse tipo aqui!?).
	- Como estamos tratando de um arquivo em disco, dependendo das adições e retiradas, ficaremos com buracos, esses buracos devem ficar
	em um vetor (local de inicio e tamanho do buraco) preferencialmente separado por um map ou algo que permita facilmente descobrir se
	um item de X tamanho caiba em algum dos nossos buracos (como vamos dividir em sessões basta ver se algum dos buracos tem a quantidade
	de sessões que precisamos? mas isso é um look linear, usar map?).
	- Devemos aceitar uma quantidade máxima de buracos, após isso devemos marcar esse fragment como "impuro" e ele só poderá ser usado para
	adicionar novos itens depois de ser "purificado" (rodar o algoritmo de otimização de espaço).
	- Essa classe deve prover funcionalidades do tipo: InsertData (direto com o arquivo, chamando a ReserveData, ou com o objeto retornado
	pela função de reservar dados/espaço?), DeleteData, GetData (from hash or direct location?), ReserveData, IsPure, Otimize, etc.
*/

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFragment
////////////////////////////////////////////////////////////////////////////////
class PacketFragment
{
public:

	// The fragment file type
	typedef uint64_t FileIdentifier;

	// The section metadata type
	struct SectionMetadata
	{
		// The section starting byte
		uint32_t sectionStartingByte;

		// The section size (in bytes)
		uint32_t sectionSize;

		// The file identifier (unnused if this is an empty section)
		FileIdentifier fileIdentifier;
	};

private:

	// The maximum allowed unused sections (this will be used to determine if this fragment is pure)
	const uint32_t FragmentMaximumUnusedSections = 20;

public:


	// Json friend functions
	friend void to_json(nlohmann::json& _json, const PacketFragment::SectionMetadata& _object);
	friend void from_json(const nlohmann::json& _json, PacketFragment::SectionMetadata& _object);

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFragment(std::string _fragmentName);
	PacketFragment(std::string _fragmentName, uint32_t _maximumSize);
	~PacketFragment();

//////////////////
// MAIN METHODS //
public: //////////

	// Insert a data into this fragment
	bool InsertData(unsigned char* _data, uint32_t _dataSize, FileIdentifier& _fileIdentifier);

	// Remove a data from this fragment
	bool DeleteData(FileIdentifier _fileIdentifier);

	// Return the data size for the given file identifier
	uint32_t GetDataSize(FileIdentifier _fileIdentifier);

	// Return a data from this fragment
	bool GetData(unsigned char* _data, FileIdentifier _fileIdentifier);

	// Return if this fragment is pure (if it has more sections than it should have)
	bool IsPure();

	// Optimize this fragment (re-arrange the data blocks)
	bool Optimize();

	// Return this fragment name
	std::string GetName();

	// Check if this fragment has an unused section of (at last) the given size
	bool HasUnusedSectionWithAtLast(uint32_t _size);

	// Rename this fragment
	bool Rename(std::string _fragmentName, std::string _extension = PacketStrings::FragmentComplementName);

private:

	// Allocate a section for the given size
	bool AllocateSection(uint32_t _size, SectionMetadata& _section);

	// Desalocate a section
	void DeallocateSection(SectionMetadata _section);

	// Write data to the data file
	bool WriteDataToFile(unsigned char* _data, uint32_t _position, uint32_t _size);

	// Read data from the file
	bool ReadDataFromFile(unsigned char* _data, uint32_t _position, uint32_t _size);

private:

	// Open/close the data file
	bool OpenDataFile();
	void CloseDataFile();

	// Read the metadata
	bool ReadMetadata();

public:

	// Save the metadata
	bool SaveMetadata();

///////////////
// VARIABLES //
private: //////

	// The fragment name
	std::string m_FragmentName;

	// The total number of files inside this fragment
	uint32_t m_FragmentTotalFiles;

	// The fragment maximum size
	uint32_t m_FragmentMaximumSize;

	// The fragment internal index counter
	uint32_t m_FragmentInternalIndexCounter;

	// The fragment data file stream
	std::fstream m_FragmentDataStream;

	// All unused sections (we will probably have at last one because the "remaining" empty space is considered one huge section)
	std::vector<SectionMetadata> m_FragmentUnusedSections;

	// The file identifier/section mapping
	std::map<FileIdentifier, SectionMetadata> m_FragmentFileMapping;
};

// Json functions
void to_json(nlohmann::json& _json, const Packet::PacketFragment::SectionMetadata& _object);
void from_json(const nlohmann::json& _json, Packet::PacketFragment::SectionMetadata& _object);

// Packet data explorer
PacketNamespaceEnd(Packet)
