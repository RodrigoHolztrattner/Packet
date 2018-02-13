////////////////////////////////////////////////////////////////////////////////
// Filename: PeonWorker.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PeonWorker.h"
#include "PeonSystem.h"

__InternalPeon::PeonWorker::PeonWorker()
{
}

__InternalPeon::PeonWorker::PeonWorker(const __InternalPeon::PeonWorker& other)
{
}

__InternalPeon::PeonWorker::~PeonWorker()
{
}

#ifdef JobWorkerDebug
static unsigned int JobWorkerId = 0;
#endif

// The thread local identifier
thread_local int			CurrentLocalThreadIdentifier;

int __InternalPeon::PeonWorker::GetCurrentLocalThreadIdentifier()
{
	return CurrentLocalThreadIdentifier;
}

void __InternalPeon::PeonWorker::SetQueueSize(unsigned int _jobBufferSize)
{
	// Initialize our concurrent queue
	// m_WorkQueue = new moodycamel::ConcurrentQueue<__InternalPeon::PeonJob*>(_jobBufferSize);
    m_WorkQueue.Initialize(_jobBufferSize);

	// Initialize our job free list
	m_JobFreeList = new __InternalPeon::PeonJob[_jobBufferSize]();
	m_JobFreeListPosition = 0;
	m_JobFreeListMax = _jobBufferSize;
}

bool __InternalPeon::PeonWorker::Initialize(__InternalPeon::PeonSystem* _ownerSystem, int _threadId, bool _mainThread)
{
	// pthread_t thread;

	// Set the thread id and owner system
	m_ThreadId = _threadId;
	m_OwnerSystem = _ownerSystem;

	//
	std::cout << "Thread with id: " << m_ThreadId << " created" << std::endl;

	// ?????????
	// Check if this worker thread is the main one
	if (!_mainThread)
	{
		// Create the new thread
		new std::thread(&PeonWorker::ExecuteThreadAux, this);
	}
	else
	{
		// Set the current local thread identifier
		CurrentLocalThreadIdentifier = m_ThreadId;
	}

	return true;
}

void __InternalPeon::PeonWorker::ExecuteThreadAux()
{
	// Set the global per thread id
	CurrentLocalThreadIdentifier = m_ThreadId;

	// Run the execute function
	while (true)
	{
		ExecuteThread(nullptr);
	}
}

bool __InternalPeon::PeonWorker::GetJob(__InternalPeon::PeonJob** _job)
{
	// Primeira coisa, vamos ver se conseguimos pegar algum work do nosso queue interno
	*_job = m_WorkQueue.Pop();
	if (*_job != nullptr)
	{
		// Conseguimos! Retornamos ele agora
		return true;
	}

	// Fast random number generator
	auto FastRand = [](){static unsigned int g_seed;g_seed = (214013*g_seed+2531011);return (g_seed>>16)&0x7FFF;};

	// Primeiramente pegamos um index aleatório de alguma thread e a array de threads
	unsigned int randomIndex = FastRand() % m_OwnerSystem->GetTotalWorkerThreads();
	__InternalPeon::PeonWorker* workers = m_OwnerSystem->GetJobWorkers();

	// Pegamos então o queue desta thread aleatória
	PeonStealingQueue* stolenQueue = workers[randomIndex].GetWorkerQueue();

	// Verificamos se não estamos roubando de nós mesmos
	if (stolenQueue == &m_WorkQueue)
	{
		return false;
	}

	// Roubamos então um work desta thread
	*_job = stolenQueue->Steal();
	if (*_job == nullptr)
	{
		// Não foi possível roubar um work desta thread, melhor parar por aqui!
		return false;
	}

	// Conseguimos roubar um work!
	return true;
}

__InternalPeon::PeonJob* __InternalPeon::PeonWorker::GetFreshJob()
{
    return m_WorkQueue.GetFreshJob();

    // Select a valid job
    PeonJob* selectedJob = &m_JobFreeList[m_JobFreeListPosition];

    // Increment the free list position
    m_JobFreeListPosition++;

    return selectedJob;
}

__InternalPeon::PeonStealingQueue* __InternalPeon::PeonWorker::GetWorkerQueue()
{
    return &m_WorkQueue;
}

void __InternalPeon::PeonWorker::Yield()
{
    std::this_thread::yield();
}

int __InternalPeon::PeonWorker::GetThreadId()
{
    return m_ThreadId;
}

void __InternalPeon::PeonWorker::ResetFreeList()
{
    m_JobFreeListPosition = 0;
}

thread_local __InternalPeon::PeonJob*	CurrentThreadJob;

void __InternalPeon::PeonWorker::ExecuteThread(void* _arg)
{
	if (__InternalPeon::PeonSystem::ThreadsBlocked())
	{
		Yield();
		return;
	}

	// Try to get a job
	__InternalPeon::PeonJob* job = nullptr;
	bool result = GetJob(&job);
	if (result)
	{

// If debug mode is on
#ifdef JobWorkerDebug

		// Print the function message
		printf("Thread with id %d will run a function", m_ThreadId);

#endif

		// Set the current job for this thread
		CurrentThreadJob = job;

		// Run the selected job
		job->RunJobFunction();

		// Finish the job
		job->Finish();
	}
	else
    {
        // Set an empty current job
        CurrentThreadJob = nullptr;

        // Give our time slice away
        Yield();
    }
}

__InternalPeon::PeonJob* __InternalPeon::PeonWorker::GetCurrentJob()
{
	return CurrentThreadJob;
}
