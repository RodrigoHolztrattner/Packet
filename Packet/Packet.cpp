// Packet.cpp : Defines the entry point for the console application.
//

#include "PacketFragment.h"
#include "PacketObject.h"
#include "PacketStringOperations.h"
#include "PacketFile.h"

#include <iostream>

/*

	- Criar um PacketFileBase que vai funcionar como um "file object", o mesmo será como espaço para receber dados.
		# Ele pode ter seu espaço alocado, não alocado ou apontado, caso não alocado podemos usar um custom allocator ou alocar no modo default quando for preciso.
		# Caso alocado (por um custom alocator ou não, tanto faz), colocaremos nele os dados lidos.
		# Caso apontado, ele aponta para um espaço de memória supostamente válido, logo os dados serão colocados lá.

		# O modo de recebimento dos dados pode ser sincrono ou assincrono, caso seja sincrono nós usaremos algum barrier para carregar assim que for possível (pelo fragment??)
		# Caso o modo seja assincrono, adicionaremos ele em uma lista (com prioridade??) que fica dentro do fragment (??) que se encarregará de carregá-lo assim que possível
		
		# O carregamento de um objeto seta uma variável interna do mesmo como ready (dirty?) e pode realizar a chamada de um callback




*/

// TODO:
/*
	- (DONE) Adicionar forma de verificar erros (retornar os erros de alguma forma)
	- (NÃO NECESSÁRIO, CASO ALGO DE ERRADO EXISTE A OPÇÃO VERBOSE DE ERROS) Log opcional de quando algo é feito usando o iterator (arquivo tal foi colocado em tal pasta, path: blabla.bla, arquivo tal foi deletado, etc)
	- (NOT DONE: POSSÍVEL MAS MEIO INÚTIL) Ver uma forma precisa de descobrir se um path é um arquivo ou dir apenas (validar tal coisa no modo iterator)
	- (DONE) Ver uma forma precisa de pegar a extensão de um arquivo (adicionar unknow? caso desconhecido)
	- (DONE) Adicionar extenções dos arquivos como um field de metadado
	- Adicionar alguma extensão para debug no modo PacketFile
	- Verificar em quais casos um novo fragment é criado (e se esses casos estão ok)
	- (DONE) Criar um arquivo que contenha todos as strings usadas (extensões, nomes de arquivos, etc)
	- (DONE) Criar um .bla conhecido por esse formato, pode ser o proprio .packet
	- (DONE?) Modificar a extensão dos nomes dos fragments
	- Criar função delete no iterator
	- Criar função move no iterator
	- Criar função de otimização no manager
*/

/*
	- Devemos utilizar a hash pois ela contem todos os fragment identifiers (m_PacketHashTableReference).
	- Teoricamente podemos mudar apenas o fragment identifier (na hash) quando for feita a otimização.
	- Devemos usar o PacketObjectManager e o PacketObjectHashTable para a otimização.
	- Preciso colocar uma função de pegar informações do file (pelo menos size).

	- A otimização deve ser dividida em 2 partes:
		1: Devemos verificar em cada fragmento se existe alguma sessão que pode ser juntada com outra (desde que elas não estejam em uso e uma termine onde a outra comece).
		2: Do ultimo fragmento até o primeiro (sem contar o primeiro), pegamos cada arquivo e tentamos inserir ele no menor espaço possível que existe entre o primeiro e o 
		fragmento anterior que começamos a busca. Repete...


*/

std::vector<std::string> Split(const std::string &txt, char ch)
{
	std::vector<std::string> result;
	size_t pos = txt.find(ch);
	unsigned int initialPos = 0;
	result.clear();

	// Decompose statement
	while (pos != std::string::npos) {
		result.push_back(txt.substr(initialPos, pos - initialPos));
		initialPos = pos + 1;

		pos = txt.find(ch, initialPos);
	}

	// Add the last one
	result.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

	// Remove blank strings
	for (unsigned int i = 0; i < result.size(); i++)
	{
		// Compare
		if (result[i].compare(" ") == 0)
		{
			// Remove this blank string
			result.erase(result.begin() + i);
			i--;
		}
	}

	return result;
}

// Load a packet object from the command line input
Packet::PacketObject* GetPacket()
{
	// Create the new packet object
	Packet::PacketObject* newPackObject = new Packet::PacketObject();

	// Load/create
	while (true)
	{
		// Print the load/create message
		std::cout << "cp <name> <size> - create a new pack with name <name>.packet and maximum fragment size <size>" << std::endl;
		std::cout << "lp <path> - load an existing pack at <path>" << std::endl;

		// Jump the line
		std::cout << std::endl;

		// Print the command line icon
		std::cout << '>';

		// Capture each command
		std::string command;
		std::getline(std::cin, command);

		// Jump the line
		std::cout << std::endl;

		// Break the command
		std::vector<std::string> commands = Split(command, ' ');

		// Check if we have at last one command
		if (commands.size() == 0)
		{
			continue;
		}

		// Verify the command
		if (commands[0] == "cp" && commands.size() == 3)
		{
			if (newPackObject->InitializeEmpty(commands[1], std::stoi(commands[2])))
			{
				break;
			}
		}
		if (commands[0] == "lp" && commands.size() == 2)
		{
			if (newPackObject->InitializeFromFile(commands[1]))
			{
				break;
			}
		}
	}

	///
	///
	///


	/*
	Packet::PacketFile* newFile = new Packet::PacketFile(newPackObject, Packet::PacketFile::DispatchType::Assync, true);
	bool result = newFile->LoadWithName("Images\\gimp.png");
	*/

	return newPackObject;
}

void Console()
{
	// Our packet object
	Packet::PacketObject& packetObject = *GetPacket();

	// Get the packet iterator
	auto iterator = packetObject.GetIterator();

	while (true)
	{
		// Print the current directory
		std::cout << iterator.GetCurrentPath() << '>';

		// Capture each command
		std::string command;
		std::getline(std::cin, command);

		// Jump the line
		std::cout << std::endl;

		// Break the command
		std::vector<std::string> commands = Split(command, ' ');
		
		// Check if we have at last one command
		if (commands.size() == 0)
		{
			continue;
		}

		//////////////
		// COMMANDS //
		//////////////

		// Info
		if (commands[0].compare("info") == 0 && commands.size() == 1)
		{
			std::cout << "  - cd <path>: Seek to the given <path>" << std::endl;
			std::cout << "  - mkdir <path>: Create a new folder into the given <path>" << std::endl;
			std::cout << "  - ls <path(optional)>: List all files and folders in the current directory or in the given <path>" << std::endl;
			std::cout << "  - put <ext:filepath> <path(optional)>: Put the <ext:filepath> file in the current directory or in the given <path>" << std::endl;
			std::cout << "  - get <filepath> <ext:path(optional)>: Get the <filepath> file and put it in the current operation system directry or in the <ext:path> location" << std::endl;
			std::cout << "  - delete <path>: Delete the file or folder on <path>" << std::endl;
			std::cout << "  - save: This MUST be called before finishing the execution to save all the data" << std::endl;

			std::cout << std::endl;
		}

		// Seek
		if (commands[0].compare("cd") == 0 && commands.size() == 2)
		{
			iterator.Seek(commands[1]);
		}

		// Mkdir
		if (commands[0].compare("mkdir") == 0 && commands.size() == 2)
		{
			iterator.MakeDir(commands[1]);
		}

		// List
		if (commands[0].compare("ls") == 0)
		{
			std::vector<std::string> result;

			if (commands.size() == 2)
			{
				result = iterator.List(commands[1]);
			}
			else
			{
				result = iterator.List();
			}

			// For each result
			for (auto& name : result)
			{
				std::cout << "   - " << name << std::endl;
			}
			if (result.size()) std::cout << std::endl;
		}

		// Put
		if (commands[0].compare("put") == 0 && commands.size() >= 2)
		{
			if (commands.size() == 2)

			{
				iterator.Put(commands[1]);
			}
			else
			{
				iterator.Put(commands[1], commands[2]);
			}
		}

		// Get
		if (commands[0].compare("get") == 0 && commands.size() >= 2)
		{
			if (commands.size() == 2)
			{
				iterator.Get(commands[1]);
			}
			else
			{
				iterator.Get(commands[1], commands[2]);
			}
		}

		// Delete
		if (commands[0].compare("delete") == 0 && commands.size() >= 2)
		{
			if (commands.size() == 2)
			{
				iterator.Delete(commands[1]);
			}
			else
			{
				// iterator.Get(commands[1], commands[2]);
			}
		}

		// Save
		if (commands[0].compare("save") == 0)
		{
			packetObject.SavePacketData();
		}

		// Exit
		if (commands[0].compare("exit") == 0)
		{
			packetObject.SavePacketData();
			exit(0);
		}

		// Optimize
		if (commands[0].compare("optimize") == 0)
		{
			// iterator.Optimize();
		}
	}
}

#include "PacketFileDataOperations.h"

int main()
{
	Console();






	// Packet::PacketObject packetObject("Wonderland", 8096);

	// auto iterator = packetObject.GetIterator();
	// iterator.Put("Old\\Packet.h");


	// preciso ver como vai ser a divisão dentro dos arquivos(em sections) e qual o tamanho delas(ou como organizar isso);

    return 0;
}

