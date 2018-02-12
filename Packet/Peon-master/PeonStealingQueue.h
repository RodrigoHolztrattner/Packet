////////////////////////////////////////////////////////////////////////////////
// Filename: PeonStealingQueue.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PeonConfig.h"
#include "PeonJob.h"
#include <vector>
#include <cstdint>
#include <atomic>

///////////////
// NAMESPACE //
///////////////

// __InternalPeon
PeonNamespaceBegin(__InternalPeon)

/////////////
// DEFINES //
/////////////


////////////
// GLOBAL //
////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PeonStealingQueue
////////////////////////////////////////////////////////////////////////////////
class PeonStealingQueue
{
public:
	PeonStealingQueue();
	PeonStealingQueue(const PeonStealingQueue&);
	~PeonStealingQueue();

	// Initialize the work stealing queue
	bool Initialize(unsigned int _bufferSize);

	// Return a valid job from our ring buffer
	PeonJob* GetFreshJob();

    // Insert a job into this queue (must be called only by the owner thread)
	void Push(PeonJob* _job);

    // Get a job from this queue (must be called only by the owner thread)
	PeonJob* Pop();

    // Try to steal a job from this queue (can be called from any thread)
	PeonJob* Steal();

    // Reset this deque (start at the initial position)
	void Reset();

private:

	// The top and bottom deque positions
	std::atomic<long int> m_Top;
	std::atomic<long int> m_Bottom;

	// The job buffer size (for the deque and the ring buffer)
	long int m_BufferSize;

	// The job ring buffer position
	long int m_RingBufferPosition;

	// The job ring buffer
	PeonJob* m_RingBuffer;

	// The deque buffer
	PeonJob** m_DequeBuffer;
};

// __InternalPeon
PeonNamespaceEnd(__InternalPeon)
