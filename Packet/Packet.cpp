// Packet.cpp : Defines the entry point for the console application.
//

#include "PacketFragment.h"
#include "PacketObject.h"
#include "PacketStringOperations.h"
#include "PacketFile.h"
#include "PacketFileRequester.h"

#include <iostream>
#include <numeric>

#include "Peon-master\Peon.h"

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
*/

// Obs: There are some "TODO"s inside the classes, I need to check those too (just use a global find w/ TODO as the keyword)

void PerformThreadedTests()
{
	// Initialize the peon system
	Peon::Initialize(4, 4096);

	// Create the new packet object
	Packet::PacketObject* newPackObject = new Packet::PacketObject();

	// Initialize the packet object
	if(!newPackObject->InitializeEmpty("ThreadedPacketTest", 67108864/4))
	{
		return;
	}

	// Initialize the packet file requester
	Packet::PacketFileRequester* packetFileRequester = new Packet::PacketFileRequester(newPackObject);

	// Get the packet iterator
	auto iterator = newPackObject->GetIterator();

	// Set the threaded index method
	packetFileRequester->UseThreadedQueue(4, [&]() {
		int threadIndex = Peon::GetCurrentWorkerIndex();
		return Peon::GetCurrentWorkerIndex();
	});

	///////////////////
	// FILE CREATION //
	///////////////////
	
	// The total number of files we will create and the temporary data with its size
	const uint32_t totalNumberFiles = 1000;
	uint32_t temporaryDataSize = 1024;
	unsigned char* temporaryData = new unsigned char[temporaryDataSize];

	// Create each file
	for (int i = 0; i < totalNumberFiles; i++)
	{
		// Create the filename
		std::string filename = "Test" + std::to_string(i) + ".txt";

		// Create the file
		bool result = iterator.Put(temporaryData, temporaryDataSize, filename);
		if (!result)
		{
			return;
		}
	}

	//////////////////////
	// FILE PREPARATION //
	//////////////////////

	// Allocate space for each future file
	Packet::FutureReference<Packet::PacketFile>* files = new Packet::FutureReference<Packet::PacketFile>[totalNumberFiles];

	//////////////////
	// JOB CREATION //
	//////////////////

	// Create the job container
	Peon::Container* jobContainer = Peon::CreateJobContainer();

	// For each created file
	for (int i = 0; i < totalNumberFiles; i++)
	{
		// Create a new loading job
		Peon::Job* newJob = Peon::CreateChildJob(jobContainer, [=]() {

			// Create the filename
			std::string filename = "Test" + std::to_string(i) + ".txt";

			// Request a new file
			bool result = packetFileRequester->RequestFile(&files[i], filename.c_str(), Packet::PacketFile::DispatchType::OnProcess, true);
			if (!result)
			{
				return;
			}
		});

		// Start the new job
		Peon::StartJob(newJob);
	}

	// Start the container job
	Peon::StartJob(jobContainer);

	// Wait for the container
	Peon::WaitForJob(jobContainer);

	// Process the request queues
	packetFileRequester->ProcessFileQueues();

	// Refresh the peon system
	Peon::ResetWorkFrame();

	// For each created file
	for (int i = 0; i < totalNumberFiles; i++)
	{
		// Release this file
		if (files[i].IsRead())
		{
			if (files[i]->IsReady())
			{
				// Create a new job
				Peon::Job* newJob = Peon::CreateChildJob(jobContainer, [=]() {

					// Release this file
					files[i]->Release();
				});

				// Start the new job
				Peon::StartJob(newJob);
			}
		}
	}

	// Start the container job
	Peon::StartJob(jobContainer);

	// Wait for the container
	Peon::WaitForJob(jobContainer);

	// Refresh the peon system
	Peon::ResetWorkFrame();

	// Process the request queues
	packetFileRequester->ProcessFileQueues();

	// Sleep a little
	// ...
}

int main()
{
	PerformThreadedTests();
    return 0;
}