// Packet.cpp : Defines the entry point for the console application.
//

#include "PacketFragment.h"
#include "PacketObject.h"
#include "PacketStringOperations.h"
#include "PacketFile.h"

#include <iostream>

// My internal TODO list (pt-br):
/*
- (DONE) Adicionar forma de verificar erros (retornar os erros de alguma forma)
- (NÃO NECESSÁRIO, CASO ALGO DE ERRADO EXISTE A OPÇÃO VERBOSE DE ERROS) Log opcional de quando algo é feito usando o iterator (arquivo tal foi colocado em tal pasta, path: blabla.bla, arquivo tal foi deletado, etc)
- (NOT DONE: POSSÍVEL MAS MEIO INÚTIL) Ver uma forma precisa de descobrir se um path é um arquivo ou dir apenas (validar tal coisa no modo iterator)
- (DONE) Ver uma forma precisa de pegar a extensão de um arquivo (adicionar unknow? caso desconhecido)
- (DONE) Adicionar extenções dos arquivos como um field de metadado
- Adicionar alguma extensão para debug no modo PacketFile
- (DONE) Verificar em quais casos um novo fragment é criado (e se esses casos estão ok)
- (DONE) Criar um arquivo que contenha todos as strings usadas (extensões, nomes de arquivos, etc)
- (DONE) Criar um .bla conhecido por esse formato, pode ser o proprio .packet
- (DONE?) Modificar a extensão dos nomes dos fragments
- (DONE) Criar função delete no iterator
- Criar função move no iterator
- (DONE) Criar função de otimização no manager
- Criar uma nova classes (PacketFileHash) que vai funcionar caso o sistema queira ter apenas uma referencia de cada recurso
em uso, dessa forma um ponteiro para o mesmo deve ser mantido registrado com a Key de entrada. Deve ser adicionado um contador
de referencia (esse possivelmente deve ficar no proprio file por causa do shutdown mas ele precisa comunicar o hash e
a storage classes)
- PacketFileLoader: Adicionar uma forma de uma thread externa realizar a ThreadedLoadRoutine()
- PacketFragment: Devemos aceitar uma quantidade máxima de buracos, após isso devemos marcar esse fragment como "impuro" e ele só poderá ser usado para
adicionar novos itens depois de ser "purificado" (rodar o algoritmo de otimização de espaço).
- PacketObjectManager (opcional): Deve permitir fazer um agrupamento de itens selecionados (para que os mesmos se encontrem proximos localmente e de rápida leitura).
- Modificar os callbacks do future resource e do packet file de forma que os mesmos sejam disparados no mesmo lugar
quando carregados.
- Ver se existe a necessidade de load/ready callbacks para o future object e o packet file.
*/

// Obs: There are some "TODO"s inside the classes, I need to check those too (just use a global find w/ TODO as the keyword)

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
			iterator.Delete(commands[1]);
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
			iterator.Optimize();
		}
	}
}

#include "PacketFileDataOperations.h"

int main()
{
	Console();
    return 0;
}