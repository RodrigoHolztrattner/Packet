////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFileQueue.h"
#include "PacketFile.h"

Packet::PacketFileQueue::PacketFileQueue()
{
}

Packet::PacketFileQueue::~PacketFileQueue()
{
}

void Packet::PacketFileQueue::Enqueue(PacketFile* _file)
{
	// Lock our mutex
	std::lock_guard<std::mutex> lock(m_Mutex);

	// Insert the file
	m_FileList.push_back(_file);
}

Packet::PacketFile* Packet::PacketFileQueue::TryDequeue()
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

uint32_t Packet::PacketFileQueue::GetSize()
{
	// Lock our mutex
	std::lock_guard<std::mutex> lock(m_Mutex);

	return m_FileList.size();
}