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

void FileCreationAndDeletion(const char* _packetName, uint32_t _packetSize, const uint32_t _totalNumberFiles, uint32_t _temporaryDataSize)
{
	// Create the new packet object
	Packet::PacketObject* newPackObject = new Packet::PacketObject();

	// Initialize the packet object
	if (!newPackObject->InitializeEmpty(_packetName, _packetSize))
	{
		return;
	}

	// Initialize the packet file requester
	Packet::PacketFileRequester* packetFileRequester = new Packet::PacketFileRequester(newPackObject);

	// Get the packet iterator
	auto iterator = newPackObject->GetIterator();

	// Set the threaded index method
	packetFileRequester->UseThreadedQueue(4, [&]() {
		return Peon::GetCurrentWorkerIndex();
	});

	///////////////////
	// FILE CREATION //
	///////////////////

	// The total number of files we will create and the temporary data with its size
	unsigned char* temporaryData = new unsigned char[_temporaryDataSize];
	memset(temporaryData, 0, sizeof(unsigned char) * _temporaryDataSize);

	// Create each file
	for (int i = 0; i < _totalNumberFiles; i++)
	{
		// Create the filename
		std::string filename = "Test" + std::to_string(i) + ".txt";

		// Create the file
		bool result = iterator.Put(temporaryData, _temporaryDataSize, filename);
		if (!result)
		{
			return;
		}
	}

	//////////////////////
	// FILE PREPARATION //
	//////////////////////

	// Allocate space for each future file
	Packet::FutureReference<Packet::PacketFile>* files = new Packet::FutureReference<Packet::PacketFile>[_totalNumberFiles * 4];

	//////////////////
	// JOB CREATION //
	//////////////////

	// Create the job container
	Peon::Container* jobContainer = Peon::CreateJobContainer();

	// For each created file
	for (int i = 0; i < _totalNumberFiles * 4; i++)
	{
		// Create a new loading job
		Peon::Job* newJob = Peon::CreateChildJob(jobContainer, [=]() {

			// Set the index
			int index = i / 4;

			// Create the filename
			std::string filename = "Test" + std::to_string(index) + ".txt";

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
	for (int i = 0; i < _totalNumberFiles * 4; i++)
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

	// Delete the temporary data
	delete[] temporaryData;
	delete[] files;
}

void PerformThreadedTests()
{
	// Initialize the peon system
	Peon::Initialize(4, 8192);

	// Create 1k files, uses 4 thread to reference those files 4k times (each file is referenced 4 times), delete all files
	FileCreationAndDeletion("ThreadedPacketTest", 67108864 / 4, 1000, 1024);

	// Other tests
	// TODO
}

int main()
{
	// Perform all tests
	PerformThreadedTests();

    return 0;
}