// Packet.cpp : Defines the entry point for the console application.
//

#include "PacketFragment.h"
#include "PacketObject.h"

int main()
{
	Packet::PacketObject packetObject;

	auto iterator = packetObject.GetIterator();
	iterator.Put("Old\\Packet.h");


	// preciso ver como vai ser a divisão dentro dos arquivos(em sections) e qual o tamanho delas(ou como organizar isso);

    return 0;
}

