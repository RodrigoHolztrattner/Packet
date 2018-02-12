////////////////////////////////////////////////////////////////////////////////
// Filename: PeonStealingQueue.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PeonStealingQueue.h"

///////////////
// NAMESPACE //
///////////////

__InternalPeon::PeonStealingQueue::PeonStealingQueue()
{
}

__InternalPeon::PeonStealingQueue::PeonStealingQueue(const __InternalPeon::PeonStealingQueue& other)
{
}

__InternalPeon::PeonStealingQueue::~PeonStealingQueue()
{
}

bool __InternalPeon::PeonStealingQueue::Initialize(unsigned int _bufferSize)
{
    // Set the size and allocate the ring buffer
    m_BufferSize = _bufferSize;
    m_RingBuffer = new PeonJob[_bufferSize];

    // Set the deque size (allocate memory for it)
    m_DequeBuffer = new PeonJob*[_bufferSize];

    // Set the initial position
    m_RingBufferPosition = 0;

    // Set the top and bottom positions
    m_Top = 0;
    m_Bottom = 0;

    return true;
}

__InternalPeon::PeonJob* __InternalPeon::PeonStealingQueue::GetFreshJob()
{
    const long int index = m_RingBufferPosition++;
    return &m_RingBuffer[(index-1u) & (m_BufferSize-1u)];
}

void __InternalPeon::PeonStealingQueue::Reset()
{
    m_RingBufferPosition = 0;
    m_Top = 0;
    m_Bottom = 0;
}

void __InternalPeon::PeonStealingQueue::Push(PeonJob* _job)
{
    long b = m_Bottom;

    // Push the job
    m_DequeBuffer[b & (m_BufferSize - 1)] = _job;

    // ensure the job is written before b+1 is published to other threads.
    // on x86/64, a compiler barrier is enough.
    std::atomic_thread_fence(std::memory_order_acq_rel);

    // Set the new bottom
    m_Bottom = b + 1;
}

__InternalPeon::PeonJob* __InternalPeon::PeonStealingQueue::Pop()
{
    long int b = m_Bottom - 1;

    // Set the new bottom
    m_Bottom.exchange(b, std::memory_order_acq_rel);

    // Deque status...
    long int t = m_Top;
    if (t <= b)
    {
        // non-empty queue
        PeonJob* job = m_DequeBuffer[b & (m_BufferSize - 1)];
        if (t != b)
        {
            // There's still more than one item left in the queue
            return job;
        }

        // This is the last item in the queue
        if (!m_Top.compare_exchange_weak(t, t + 1, std::memory_order_acq_rel))
        {
            // failed race against steal operation
            job = nullptr;
        }

        m_Bottom = t + 1;
        return job;
    }
    else
    {
        // Deque was already empty
        m_Bottom = t;
        return nullptr;
    }
}

__InternalPeon::PeonJob* __InternalPeon::PeonStealingQueue::Steal()
{
    long int t = m_Top;

    // ensure that top is always read before bottom.
    // loads will not be reordered with other loads on x86, so a compiler barrier is enough.
    std::atomic_thread_fence(std::memory_order_acq_rel);

    // Deque status...
    long int b = m_Bottom;
    if (t < b)
    {
        // non-empty queue
        PeonJob* job = m_DequeBuffer[t & (m_BufferSize - 1)];

        // the interlocked function serves as a compiler barrier, and guarantees that the read happens before the CAS.
        if (!m_Top.compare_exchange_weak(t, t + 1, std::memory_order_acq_rel)) // Mudar para strong?
        {
            // a concurrent steal or pop operation removed an element from the deque in the meantime.
            return nullptr;
        }

        return job;
    }
    else
    {
        // empty queue
        return nullptr;
    }
}
