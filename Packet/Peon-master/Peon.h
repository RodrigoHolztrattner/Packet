////////////////////////////////////////////////////////////////////////////////
// Filename: PeonJob.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PeonConfig.h"
#include "PeonSystem.h"
#include "PeonWorker.h"
#include "PeonJob.h"

/////////////
// DEFINES //
/////////////

///////////////
// NAMESPACE //
///////////////

// Peon
PeonNamespaceBegin(Peon)

////////////
// GLOBAL //
////////////

typedef __InternalPeon::PeonWorker	Worker;
typedef __InternalPeon::PeonJob		Job;
typedef __InternalPeon::PeonJob		Container;

// Our peon system global instance
static __InternalPeon::GlobalInstance<__InternalPeon::PeonSystem> PeonSystemGlobalInstance;

// Initialize the peon system
template <class ThreadUserDatType = unsigned char>
static bool Initialize(unsigned int _numberWorkerThreads, unsigned int _jobBufferSize, bool _useUserData = false)
{
	// Initialize the peon system
	return PeonSystemGlobalInstance->Initialize<ThreadUserDatType>(_numberWorkerThreads, _jobBufferSize, _useUserData);
}

// Create an independent job
static Job* CreateJob(std::function<void()> _function)
{
	return PeonSystemGlobalInstance->CreateJob(_function);
}

// Create a child job
static Job* CreateChildJob(std::function<void()> _function)
{
	return PeonSystemGlobalInstance->CreateChildJob(_function);
}

// Create a child job
static Job* CreateChildJob(Job* _parentJob, std::function<void()> _function)
{
	return PeonSystemGlobalInstance->CreateChildJob(_parentJob, _function);
}

// Create a job container
static Container* CreateJobContainer()
{
	__InternalPeon::PeonSystem* jobSystem = (__InternalPeon::PeonSystem*)PeonSystemGlobalInstance.GetInstance(); // Remove the const
	return PeonSystemGlobalInstance->CreateJob([=] { jobSystem->JobContainerHelper(nullptr); });
}

// Start a given job
static void StartJob(Job* _job)
{
	PeonSystemGlobalInstance->StartJob(_job);
}

// Wait for a given job
static void WaitForJob(Job* _job)
{
	PeonSystemGlobalInstance->WaitForJob(_job);
}

// Return the current job for the actual context
static Job* GetCurrentJob()
{
	return __InternalPeon::PeonWorker::GetCurrentJob();
}

// Return the current worker for the actual context
static Worker* GetCurrentWorker()
{
	return __InternalPeon::PeonSystem::GetCurrentPeon();
}

// Return the current worker index for the actual context
static int GetCurrentWorkerIndex()
{
	return __InternalPeon::PeonWorker::GetCurrentLocalThreadIdentifier();
}

// Return the total number of worker threads
static int GetTotalWorkers()
{
	return PeonSystemGlobalInstance->GetTotalWorkerThreads();
}

// Return a peon user data by index
template <class PeonUserDatType>
static PeonUserDatType* GetUserData(unsigned int _workerIndex)
{
	return PeonSystemGlobalInstance->GetUserData<PeonUserDatType>(_workerIndex);
}

// Return the current peon worker user data
template <class PeonUserDatType>
static PeonUserDatType* GetUserData()
{
	return PeonSystemGlobalInstance->GetUserData<PeonUserDatType>(GetCurrentWorkerIndex());
}

// Block all threads execution
static void BlockWorkerExecution()
{
	PeonSystemGlobalInstance->BlockThreadsStatus(true);
}

// Release all threads execution
static void ReleaseWorkerExecution()
{
	PeonSystemGlobalInstance->BlockThreadsStatus(false);
}

// Reset the work frame
static void ResetWorkFrame()
{
	PeonSystemGlobalInstance->ResetWorkerFrame();
}

// Peon
PeonNamespaceEnd(Peon)
