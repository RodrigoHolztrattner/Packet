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

TODO:
/*
	- Adicionar forma de verificar erros (retornar os erros de alguma forma)
	- Log opcional de quando algo é feito usando o iterator (arquivo tal foi colocado em tal pasta, path: blabla.bla, arquivo tal foi deletado, etc)
	- Ver uma forma precisa de descobrir se um path é um arquivo ou dir apenas (validar tal coisa no modo iterator)
	- Ver uma forma precisa de pegar a extensão de um arquivo (adicionar unknow? caso desconhecido)
	- Adicionar extenções dos arquivos como um field de metadado
	- Adicionar alguma extensão para debug no modo PacketFile
	- Verificar em quais casos um novo fragment é criado (e se esses casos estão ok)
	- Criar um arquivo que contenha todos as strings usadas (extensões, nomes de arquivos, etc)
	- Criar um .bla conhecido por esse formato, pode ser o proprio .packet
	- Modificar a extensão dos nomes dos fragments
	- Criar função delete no iterator
	- Criar função move no iterator
	- Criar função de otimização no manager
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
		std::cout << "cp <name> <size> - create a new pack with name <name> and maximum fragment size <size>" << std::endl;
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

	Packet::PacketFile* newFile = new Packet::PacketFile(newPackObject, Packet::PacketFile::DispatchType::Assync, true);
	bool result = newFile->LoadWithName("Images\\gimp.png");

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

		// Save
		if (commands[0].compare("save") == 0)
		{
			if (commands.size() == 2)
			{
				packetObject.SavePacketData(commands[1]);
			}
			else
			{
				packetObject.SavePacketData();
			}
			
		}

		// Exit
		if (commands[0].compare("exit") == 0)
		{
			packetObject.SavePacketData();
			exit(0);
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

