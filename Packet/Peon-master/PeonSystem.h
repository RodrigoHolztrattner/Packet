////////////////////////////////////////////////////////////////////////////////
// Filename: PeonSystem.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PeonConfig.h"
#include <atomic>
#include <vector>
#include "PeonJob.h"
#include "PeonWorker.h"

/////////////
// DEFINES //
/////////////

////////////
// GLOBAL //
////////////

///////////////
// NAMESPACE //
///////////////

// __InternalPeon
PeonNamespaceBegin(__InternalPeon)

////////////////////////////////////////////////////////////////////////////////
// Class name: PeonSystem
////////////////////////////////////////////////////////////////////////////////
class PeonSystem
{
public:
	PeonSystem();
	PeonSystem(const PeonSystem&);
	~PeonSystem();

	inline unsigned int pow2roundup(unsigned int x)
    {
        if (x < 0)
            return 0;
        --x;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        return x+1;
    }

	// Initialize the job system
	template <class ThreadUserDatType = unsigned char>
	bool Initialize(unsigned int _numberWorkerThreads, unsigned int _jobBufferSize, bool _useUserData = false)
	{
		// Block all threads
		BlockThreadsStatus(true);

		// Make sure we are working with a buffer size power of 2
		_jobBufferSize = pow2roundup(_jobBufferSize);

		// Save the number of worker threads
		m_TotalWokerThreads = _numberWorkerThreads;

		// Alocate space for all threads
		m_JobWorkers = new PeonWorker[_numberWorkerThreads];

		// Set the queue size for each worker thread (WE CANT DO THIS AND INITIALIZE AT THE SAME TIME!)
		for (unsigned int i = 0; i < _numberWorkerThreads; i++)
		{
			m_JobWorkers[i].SetQueueSize(_jobBufferSize);
		}

		// Create the thread user data
		for (unsigned int i = 0; i < _numberWorkerThreads && _useUserData; i++)
		{
			// Create the thread user data
			ThreadUserDatType* threadUserData = new ThreadUserDatType();
			m_ThredUserData.push_back(threadUserData);
		}

		// Initialize all threads
		for (unsigned int i = 0; i < _numberWorkerThreads; i++)
		{
			m_JobWorkers[i].Initialize(this, i, i == 0); // Check for the first thread (the main one)
		}

		// Unblock all threads
		BlockThreadsStatus(false);

		return true;
	}

	// Return the thread user data by thread index
	template <class ThreadUserDatType>
	ThreadUserDatType* GetUserData(unsigned int _threadIndex)
	{
		return (ThreadUserDatType*)m_ThredUserData[_threadIndex];
	}

	// Return the total worker threads
	unsigned int GetTotalWorkerThreads()
	{
		return m_TotalWokerThreads;
	}

	// Return the job worker array
	PeonWorker* GetJobWorkers()
	{
		return m_JobWorkers;
	}

	// Reset the actual worker frame
	void ResetWorkerFrame();

	// Job container creation helper
	void JobContainerHelper(void* _data) {}

	////////////
	// STATIC //
	////////////

	// Create a job
	static PeonJob* CreateJob(std::function<void()> _function);

	// Create a job as child
	static PeonJob* CreateChildJob(std::function<void()> _function)
	{
		return CreateChildJob(PeonWorker::GetCurrentJob(), _function);
	}

	// Create a job as child for the given parent job
	static PeonJob* CreateChildJob(PeonJob* _parentJob, std::function<void()> _function);

	// Run a job
	static void StartJob(PeonJob* _job);

	// Wait for a job to continue
	static void WaitForJob(PeonJob* _job);

	// REMOVER ESSAS FUNÇÕES OU USAR DEBUG #DEFINE
	static void BlockThreadsStatus(bool _status);
	static bool ThreadsBlocked();

	// Return the current thread executing this code
	static PeonWorker* GetCurrentPeon()
	{
		return &PeonSystem::m_JobWorkers[CurrentThreadIdentifier];
	}

	// Return the default worker thread
	__InternalPeon::PeonWorker* GetDefaultWorkerThread()
	{
		return &m_JobWorkers[0];
	}

	////////////
	////////////
	////////////

private:



	// The total of worker threads
	unsigned int m_TotalWokerThreads;

	// The worker thread array
	static __InternalPeon::PeonWorker* m_JobWorkers;

	// The thread user data
	std::vector<void*> m_ThredUserData;
};


// __InternalPeon
PeonNamespaceEnd(__InternalPeon)
