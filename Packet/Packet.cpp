// Packet.cpp : Defines the entry point for the console application.
//

#include "PacketFragment.h"
#include "PacketObject.h"
#include "PacketStringOperations.h"

#include <iostream>

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

