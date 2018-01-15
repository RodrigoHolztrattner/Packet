////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileLoaderQueue.h"
#include "PacketFile.h"

Packet::PacketFileLoaderQueue::PacketFileLoaderQueue()
{
}

Packet::PacketFileLoaderQueue::~PacketFileLoaderQueue()
{
}

void Packet::PacketFileLoaderQueue::Enqueue(PacketFile* _file)
{
	// Lock our mutex
	std::lock_guard<std::mutex> lock(m_Mutex);

	// Insert the file
	m_FileList.push_back(_file);
}

Packet::PacketFile* Packet::PacketFileLoaderQueue::TryDequeue()
{
	// Lock our mutex
	std::lock_guard<std::mutex> lock(m_Mutex);

	// Check for empty
	if(m_FileList.empty())
	{
		return nullptr;
	}

	// Get the file and remove it from the list
	PacketFile* file = m_FileList.front();
	m_FileList.pop_front();

	return file;
}

uint32_t Packet::PacketFileLoaderQueue::GetSize()
{
	// Lock our mutex
	std::lock_guard<std::mutex> lock(m_Mutex);

	return m_FileList.size();
}