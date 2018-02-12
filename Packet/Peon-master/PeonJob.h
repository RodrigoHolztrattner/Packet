////////////////////////////////////////////////////////////////////////////////
// Filename: PeonJob.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PeonConfig.h"
#include <atomic>
#include <functional>

/////////////
// DEFINES //
/////////////

// Return if a job is done
#define HasJobCompleted(job)	((job->GetTotalUnfinishedJobs()) <= 0)

///////////////
// NAMESPACE //
///////////////

// __InternalPeon
PeonNamespaceBegin(__InternalPeon)

// WorkerThread class
class PeonWorker;

////////////
// GLOBAL //
////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PeonJob
////////////////////////////////////////////////////////////////////////////////
class PeonJob
{
public:
	PeonJob();
	PeonJob(const PeonJob&);
	~PeonJob();

	// Initialize the job
	bool Initialize();

	// Set the job function (syntax: (*MEMBER, &MEMBER::FUNCTION, DATA))
	void SetJobFunction(PeonJob* _parentJob, std::function<void()> _function);

	// Finish this job
	void Finish();

	// Run the job function
	void RunJobFunction();

	// Return the parent job
	PeonJob* GetParent();

	// Set the worker thread
	void SetWorkerThread(PeonWorker* _workerThread);

	// Return the worker thread
	PeonWorker* GetWorkerThread();

	// Set the parent job
	void SetParentJob(PeonJob* _job);

	// Return the number of unfinished jobs
	int32_t GetTotalUnfinishedJobs();

protected:

	// The job function and data
	std::function<void()> m_Function;

	// The parent job
	PeonJob* m_ParentJob;

	// The current worker thread
	PeonWorker* m_CurrentWorkerThread;

public: // Arrumar public / private

	// The number of unfinished jobs
	std::atomic<int> m_UnfinishedJobs;
};

/*
static bool IsEmptyJob(PeonJob* _job)
{
	return _job == nullptr ? true : false;
}
*/

// __InternalPeon
PeonNamespaceEnd(__InternalPeon)
