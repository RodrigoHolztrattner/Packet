////////////////////////////////////////////////////////////////////////////////
// Filename: SmallPack.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"

#include <mutex>
#include <functional>
#include <vector>

/////////////
// DEFINES //
/////////////

// Packet data explorer
PacketNamespaceBegin(Packet)

template <typename ObjectType>
class MultipleQueue
{
public:

	// Constructor / destructor
	MultipleQueue()
	{
		// Set our initial data
		m_UseThreadQueue = false;
		m_MaximumTotalThreadedQueues = 0;
	}
	~MultipleQueue() {}

public:

	// Insert an object
	void Insert(ObjectType _object)
	{
		// Check if we should use a thread queue and if the thread index method was set
		if (m_UseThreadQueue && m_ThreadIndexMethod)
		{
			// Get the thread index we should use
			uint32_t threadIndex = m_ThreadIndexMethod();

			// Check if the thread index is valid
			if (threadIndex >= 0 && threadIndex < m_MaximumTotalThreadedQueues)
			{
				// Insert this file into the selected queue (no need to use mutex because each thread will access only
				// their respective queue
				m_ThreadedRequestQueues[threadIndex].push_back(_object);
			}
		}
		// Use the base queue
		else
		{
			// Lock our mutex (only one thread allowed from there)
			std::lock_guard<std::mutex> guard(m_Mutex);

			// Insert this file into the base queue
			m_BaseRequestQueue.push_back(_object);
		}
	}

	// Return a vector with all objects (this requires synchronization and isn't thread safe)
	std::vector<ObjectType> GetAll()
	{
		// Lock our mutex (only one thread allowed from there)
		std::lock_guard<std::mutex> guard(m_Mutex);

		// Our return vector
		std::vector<ObjectType> returnVector;

		// Append the base queue to the result vector
		returnVector.insert(returnVector.end(), m_BaseRequestQueue.begin(), m_BaseRequestQueue.end());

		// Clear the base object queue
		if (_clear) m_BaseRequestQueue.clear();

		// If we should use the threaded queues
		if (m_UseThreadQueue)
		{
			// For each threaded queue
			for (int i = 0; i<m_MaximumTotalThreadedQueues; i++)
			{
				// Get a reference to this queue
				auto& threadedQueue = m_ThreadedRequestQueues[i];

				// Append this queue to the result vector
				returnVector.insert(returnVector.end(), threadedQueue.begin(), threadedQueue.end());

				// Clear this request queue
				if (_clear) threadedQueue.clear();
			}
		}
		
		return returnVector;
	}

	// Process all objects (this requires synchronization and isn't thread safe)
	void ProcessAll(std::function<void(ObjectType& _object)> _processMethod, bool _clear = false)
	{
		// Lock our mutex (only one thread allowed from there)
		std::lock_guard<std::mutex> guard(m_Mutex);

		// For each object on the base queue
		for (auto& object : m_BaseRequestQueue)
		{
			// Process this object
			_processMethod(object);
		}

		// Clear the base object queue
		if(_clear) m_BaseRequestQueue.clear();

		// If we should use the threaded queues
		if (m_UseThreadQueue)
		{
			// For each threaded queue
			for (int i = 0; i<m_MaximumTotalThreadedQueues; i++)
			{
				// Get a reference to this queue
				auto& threadedQueue = m_ThreadedRequestQueues[i];

				// For each object inside this queue
				for (auto& object : threadedQueue)
				{
					// Process this object
					_processMethod(object);
				}

				// Clear this request queue
				if (_clear) threadedQueue.clear();
			}
		}
	}

	void Clear()
	{
		// Lock our mutex (only one thread allowed from there)
		std::lock_guard<std::mutex> guard(m_Mutex);

		// Clear the base object queue
		m_BaseRequestQueue.clear();

		// If we should use the threaded queues
		if (m_UseThreadQueue)
		{
			// For each threaded queue
			for (int i = 0; i<m_MaximumTotalThreadedQueues; i++)
			{
				// Get a reference to this queue
				auto& threadedQueue = m_ThreadedRequestQueues[i];

				// Clear this request queue
				if (_clear) threadedQueue.clear();
			}
		}
	}

	// Set to use multiple thread queues
	bool AllowThreadedAccess(uint32_t _totalNumberMaximumThreads, std::function<uint32_t()> _threadIndexMethod)
	{
		// Check if the number of thread is at last 1
		if (_totalNumberMaximumThreads <= 0)
		{
			return false;
		}

		// Set the threaded data
		m_MaximumTotalThreadedQueues = _totalNumberMaximumThreads;
		m_ThreadIndexMethod = _threadIndexMethod;

		// Allocate enough queues for all threads
		m_ThreadedRequestQueues = new std::vector<ObjectType>[m_MaximumTotalThreadedQueues];

		// Set that we use thread queues
		m_UseThreadQueue = true;

		return true;
	}

private:

	// If we are using the thread queue mode (we will use multiples arrays
	// (one for each thread) to allow multiple simultaneous access without mutex uses)
	bool m_UseThreadQueue;

	// The maximum of total threaded queues
	uint32_t m_MaximumTotalThreadedQueues;

	// The threaded index method <used to retrieve the current running thread index>
	std::function<uint32_t()> m_ThreadIndexMethod;

	// The mutex we will use to secure thread safe
	std::mutex m_Mutex;

	// The base file request queue
	std::vector<ObjectType> m_BaseRequestQueue;

	// The threaded queues
	std::vector<ObjectType>* m_ThreadedRequestQueues;
};

// Packet data explorer
PacketNamespaceEnd(Packet)