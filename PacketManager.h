////////////////////////////////////////////////////////////////////////////////
// Filename: PacketManager.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <map>

#include "PacketString.h"
#include "PacketFile.h"
#include "PacketDirectory.h"
#include "PacketIndex.h"

#include "..\NamespaceDefinitions.h"
#include "..\HashedString.h"

#include "PacketLoader.h"
#include "PacketIndexLoader.h"

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
NamespaceBegin(Packet)

////////////////
// FORWARDING //
////////////////

class PacketManager;

////////////////
// STRUCTURES //
////////////////

/*
	// Editor

	=> Seek:

		- Posso mudar o diret�rio virtual que estou quando quiser.
		- Posso indicar um caminho para seguir.
		- Posso pedir para voltar � origem.

	=> Procura:

		- Posso procurar no diret�rio indicado (se n�o indicado, usa a raiz).
		- Posso especificar se deve olhar em subpastas.
		- Procurar pelo nome retorna uma lista com todas as possibilidades.
		- Posso procurar pelo first match (caso tenha certeza que s� existe um arquivo com aquele nome) e ele retorna o primeiro encontrado.

	=> Inserir:

		- Ao inserir devemos sempre inserir em uma pasta, a default � a pasta raiz.
	
	=> Remover:

		- Mesma defini��o da parte inserir.

	// Runtime

		- Cada arquivo possuir� um identificador �nico, o qual ser� atribuido na hora da inser��o, o mesmo sera liberado quando este arquivo for
		removido.
		- Cada pasta tamb�m ter� um identificador �nico o qual ser� atribuido na hora da cria��o.
		- Podemos a qualquer momento descobrir qual � o identificador de um arquivo.
		- N�o � permitido o uso de multithread na leitura, este deve ser utilizado externalmente e deve ser sincronizado.

*/

/*

	- Adicionamos um objeto fazendo referencia � algum arquivo existente em disco.
	- Geramos um identificador unico para esse arquivo, quando quisermos carregar esse arquivo independente dele estar no vault ou n�o, usaremos este objeto.
	- Externamente quando queremos fazer referencia � um arquivo, podemos usar diretamente o identifcador unico dele OU perguntar utilizando o caminho dentro do vault qual � o ID do mesmo,
	dest� forma devemos fazer com que por exemplo o gerenciador de texturas fa�a o cruzamento do caminho de cada grupo com o ID correspondente do recurso, assim existe uma etapa de configura��o
	(pegar os ids) e a normal (runtime) onde usaremos os identificadores para fazer referencia ao arquivo.

*/

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketManager
////////////////////////////////////////////////////////////////////////////////
class PacketManager : public PacketLoader
{
public:

	static const uint32_t RootDirectory = -1;
	static const char Separator = '/';

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketManager();
	~PacketManager();

//////////////////
// MAIN METHODS //
public: //////////

	// Initialize
	bool Initialize(const char* _rootFileName);

	// Release this image
	void Release();

public:

//////////
// SEEK //
public: //

	// Seek the directory
	PacketDirectory* SeekDir(const char* _dirPath);
	PacketDirectory* SeekDir(uint32_t _dirIdentifier);

	/*
		- Posso mudar o diret�rio virtual que estou quando quiser.
		- Posso indicar um caminho para seguir.
		- Posso pedir para voltar � origem.
	*/

//////////
// FIND //
//////////

	// Find //TODO: permitir retornar uma lista ou apenas um no caso de first match
	PacketFile* FindFile(const char* _fileName, PacketDirectory* _currentDir, bool _recursive);
	PacketFile* FindFile(const char* _fileName, bool _recursive);
	PacketFile* FindFile(uint32_t _fileIdentifier);

	
	/*
		- Posso procurar no diret�rio indicado(se n�o indicado, usa a raiz).
		- Posso especificar se deve olhar em subpastas.
		- Procurar pelo nome retorna uma lista com todas as possibilidades.
		- Posso procurar pelo first match(caso tenha certeza que s� existe um arquivo com aquele nome) e ele retorna o primeiro encontrado.
	*/

////////////
// CREATE //
public: ////

	// Create a new dir (or return an existing one)
	PacketDirectory* CreateDir(const char* _dirName, PacketDirectory* _currentDir = nullptr);

	// Create a new file
	PacketFile* CreateFile(const char* _filePath, const char* _externalRawPath, PacketDirectory* _currentDir = nullptr);

	/*
		- Ao inserir devemos sempre inserir em uma pasta, a default � a pasta raiz.
	*/

////////////
// REMOVE //
public: ////

	/*
		- Mesma defini��o da parte inserir.
	*/

///////////
// TOOLS //
private: //

	// Find a folder using a path name
	PacketDirectory* FindFolderByPath(PacketDirectory* _fromFolder, const char* _dirPath, uint32_t _dirPathSize, bool _createIfDontExist = false);

	// Find the next name inside a formated path
	const char* FindNameInFormatedPath(const char* _formatedPath, uint32_t& _currentPosition, uint32_t& _stringSize, uint32_t& _nextStringPosition, uint32_t _maxSize);

	// Find the file name inside a formated path
	const char* FindFileNameInFormatedPath(const char* _formatedPath, uint32_t& stringSize);

	// Find a folder inside another using a name for reference
	PacketDirectory* FindFolderInside(PacketDirectory* _fromFolder, const char* _folderName, uint32_t _folderNameSize, bool _recursive = false);

	// Find a file info inside a folder using a name for reference
	PacketFile* FindFileInfoInside(PacketDirectory* _fromFolder, const char* _fileName, uint32_t _fileNameSize, bool _recursive = false);

	// Create a new folder inside a given one
	PacketDirectory* CreateFolderAux(PacketDirectory* _fromFolder, const char* _withName, uint32_t _nameSize);

	// Create a new file inside a given folder
	PacketFile* CreateFileAux(PacketDirectory* _fromFolder, const char* _withName, const char* _withExternalRawPath, uint32_t _nameSize);

private:

	// Return a new valid folder id
	uint32_t GetValidFolderIdentifier();

	// Return a new valid file id
	uint32_t GetValidFileIdentifier();

	// Free a folder identifier
	void FreeFolderIdentifier(uint32_t _identifier);

	// Free a file identifier
	void FreeFileIdentifier(uint32_t _identifier);

///////////////
// VARIABLES //
private: //////

	// The root folder
	PacketDirectory m_RootFolder;

	// Our index data
	PacketIndex m_IndexData;

	// The packet index loader
	PacketIndexLoader m_IndexLoader;
};

// Packet data explorer
NamespaceEnd(Packet)