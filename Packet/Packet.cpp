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

int main()
{
	PacketSystem packetSystem;
	packetSystem.Initialize("Data", OperationMode::Condensed);

	packetSystem.ConstructPacket();
    return 0;
}