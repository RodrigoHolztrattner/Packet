// Packet.cpp : Defines the entry point for the console application.
//

#include "Packet.h"

#include <iostream>

/*
	=> Arquivos na raiz do projeto que eu preciso ter (arquivos ocultos):

		- Lista com todos os arquivos e pastas que fazem parte do sistema, essa lista deve ser atualizada sempre que entrarmos no edit mode.
		- Lista com o cat�logo 

	=> O arquivo com as referencias s� deve ser checado quando o arquivo em si for "especionado" (sei la como se fala) usando um validador
*/

/*
	- Um arquivo pode comportar que seja editado ou n�o
*/

/*
	=> Resource
	=> Resource reference
	=> Resource temporal reference

*/

/*
	-> Carregar arquivos igual ao Peasant
	-> Poder criar um resource diretamente sem ter que carreg�-lo
	-> Poder salvar um resource (pode precisar de flags especiais)
	-> Poder editar um resource (pode precisar de flags especiais)
*/

#include "PacketSystem.h"

PacketUsingDevelopmentNamespace(Packet)

#include <fstream>

int main()
{
	PacketSystem packetSystem;
	packetSystem.Initialize("Data", OperationMode::Edit);

	//

	// Open the owning file in read mode
	Path path;
	std::ofstream file("Data\\test.txt", std::ios::binary);

	path = "Data\\gems prices.png";
	file.write((char*)&path, sizeof(Path));

	path = "Data\\Textures\\linhas terrain unreal.png";
	file.write((char*)&path, sizeof(Path));

	file.close();

	//

	bool result = packetSystem.GetReferenceManager()->ValidateFileReferences("Data\\test.txt", PacketReferenceManager::ReferenceFixer::MatchAll);

	// packetSystem.GetReferenceManager()->RegisterFileReference("Data\\test.txt", "Data\\gems prices.png", 0);
	// packetSystem.GetReferenceManager()->RegisterFileReference("Data\\test.txt", "Data\\Textures\\linhas terrain unreal.png", sizeof(Path));

	//

	packetSystem.ConstructPacket();
    return 0;
}