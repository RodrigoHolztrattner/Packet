////////////////////////////////////////////////////////////////////////////////
// Filename: PeonSystem.cpp
////////////////////////////////////////////////////////////////////////////////

#include "PeonSystem.h"
#include "PeonWorker.h"

__InternalPeon::PeonSystem::PeonSystem()
{
	// Set the initial data
	// ...
}

__InternalPeon::PeonSystem::PeonSystem(const __InternalPeon::PeonSystem& other)
{
}

__InternalPeon::PeonSystem::~PeonSystem()
{
}

// The worker thread array
__InternalPeon::PeonWorker* __InternalPeon::PeonSystem::m_JobWorkers = nullptr;

__InternalPeon::PeonJob* __InternalPeon::PeonSystem::CreateJob(std::function<void()> _function)
{
    // Get the default worker thread
    PeonWorker* workerThread = PeonSystem::GetCurrentPeon();

    // Get a fresh job
    PeonJob* freshJob = workerThread->GetFreshJob();

    // Initialize the job
    freshJob->Initialize();

    // Set the job worker thread
    freshJob->SetWorkerThread(workerThread);

    // Set the job function
    freshJob->SetJobFunction(nullptr, _function);

    // Return the new job
    return freshJob;
}

__InternalPeon::PeonJob* __InternalPeon::PeonSystem::CreateChildJob(PeonJob* _parentJob, std::function<void()> _function)
{
    // Não preciso me preocupar com o parent job sendo deletado ou liberando waits caso exista concorrencia pois seguimos da seguinte lógica, apenas o job atual pode criar jobs
    // filhos, e pode apenas adicionar elas para si mesmo, logo se estamos aqui quer dizer que o parent job ainda tem no minimo um trabalho restante (que é adicionar esse job)
    // e por consequencia ele não será deletado ou liberará algum wait.

    // Atomic increment the number of unfinished jobs of our parent
    _parentJob->m_UnfinishedJobs++;

    // Get the worker thread from the parent
    PeonWorker* workerThread = PeonSystem::GetCurrentPeon();

    // Get a fresh job
    PeonJob* freshJob = workerThread->GetFreshJob();

    // Initialize the job
    freshJob->Initialize();

    // Set the job function
    freshJob->SetJobFunction(_parentJob, _function);

    // Set the job parent
    freshJob->SetParentJob(_parentJob);

    // Return the new job
    return freshJob;
}

void __InternalPeon::PeonSystem::StartJob(__InternalPeon::PeonJob* _job)
{
	// Get the worker thread for this job
	__InternalPeon::PeonWorker* workerThread = _job->GetWorkerThread();

	// Insert the job into the worker thread queue
	workerThread->GetWorkerQueue()->Push(_job);
}

void __InternalPeon::PeonSystem::WaitForJob(__InternalPeon::PeonJob* _job)
{
	// Get the job worker thread
	__InternalPeon::PeonWorker* workerThread = _job->GetWorkerThread();

	// wait until the job has completed. in the meantime, work on any other job.
	while (!HasJobCompleted(_job))
	{
		// Try to preempt another job (or just yield)
		workerThread->ExecuteThread(nullptr);
	}
}

void __InternalPeon::PeonSystem::ResetWorkerFrame()
{
	// For each worker
	for (unsigned int i = 0; i < m_TotalWokerThreads; i++)
	{
		// Reset the free list
		m_JobWorkers[i].ResetFreeList();
	}
}

bool threadsBlocked = false;

void __InternalPeon::PeonSystem::BlockThreadsStatus(bool _status)
{
	threadsBlocked = _status;
}

bool __InternalPeon::PeonSystem::ThreadsBlocked()
{
	return threadsBlocked;
}

