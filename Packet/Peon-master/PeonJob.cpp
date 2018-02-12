////////////////////////////////////////////////////////////////////////////////
// Filename: PeonJob.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PeonJob.h"
#include "PeonWorker.h"

///////////////
// NAMESPACE //
///////////////

__InternalPeon::PeonJob::PeonJob()
{
}

__InternalPeon::PeonJob::PeonJob(const PeonJob& other)
{
}

__InternalPeon::PeonJob::~PeonJob()
{
}

bool __InternalPeon::PeonJob::Initialize()
{
    // Set the initial data
    m_CurrentWorkerThread = nullptr;
    m_ParentJob = nullptr;
    m_UnfinishedJobs = 1;

    return true;
}

void __InternalPeon::PeonJob::SetJobFunction(PeonJob* _parentJob, std::function<void()> _function)
{
    // Set the function and the data
    m_Function = _function;

    // Set the current worker thread
    m_ParentJob = _parentJob;
}

void __InternalPeon::PeonJob::Finish()
{
	// Decrement the number of unfinished jobs
	m_UnfinishedJobs--;
	const int32_t unfinishedJobs = m_UnfinishedJobs;

	// Check if there are no jobs remaining
	if (!unfinishedJobs)
	{
		// If we dont have any remaining jobs, we can decrement the number of jobs from our parent
		// (if we have one).
		if (m_ParentJob != nullptr)
		{
			// Call the finish job for our parent
			m_ParentJob->Finish();
		}
	}
}

void __InternalPeon::PeonJob::RunJobFunction()
{
    m_Function();
}

// Return the parent job
__InternalPeon::PeonJob* __InternalPeon::PeonJob::GetParent()
{
    return m_ParentJob;
}

void __InternalPeon::PeonJob::SetWorkerThread(PeonWorker* _workerThread)
{
    m_CurrentWorkerThread = _workerThread;
}

__InternalPeon::PeonWorker* __InternalPeon::PeonJob::GetWorkerThread()
{
    // Set the current job
    PeonJob* currentJob = this;

    // Check if this job is the root one
    while (currentJob->GetParent() != nullptr)
    {
        // Iterate until we find the root job
        currentJob = currentJob->GetParent();
    }

    return currentJob->m_CurrentWorkerThread;
}

void __InternalPeon::PeonJob::SetParentJob(PeonJob* _job)
{
    m_ParentJob = _job;
}

int32_t __InternalPeon::PeonJob::GetTotalUnfinishedJobs()
{
    return m_UnfinishedJobs;
}
