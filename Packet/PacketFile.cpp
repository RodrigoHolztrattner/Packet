////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFile.h"
#include "PacketObject.h"
#include "PacketFileLoader.h"

Packet::PacketFile::PacketFile(PacketFragment::FileIdentifier _fileIdentifier, DispatchType _dispatchType, bool _delayAllocation)
{
	// Set our initial data
	m_FileIdentifier = _fileIdentifier;
	m_DispatchType = _dispatchType;
	m_DelayAllocation = _delayAllocation;
	m_IsReady = false;
	m_IsDirty = true;
	m_Data = nullptr;
}

Packet::PacketFile::~PacketFile()
{
	// Check if we should deallocate our data
	if(m_Data != nullptr)
	{
		DeallocateMemory(m_Data);
		m_Data = nullptr;
	}
}

void Packet::PacketFile::SetLoadCallback(std::function<void()> _loadCallback)
{
	m_LoadCallback = _loadCallback;
}

Packet::PacketFile::DispatchType Packet::PacketFile::GetDispatchType()
{
	return m_DispatchType;
}

bool Packet::PacketFile::IsReady()
{
	return m_IsReady && !m_ErrorObject.IsSet();
}

bool Packet::PacketFile::AllocationIsDelayed()
{
	return m_DelayAllocation;
}

bool Packet::PacketFile::IsDirty()
{
	return m_IsDirty;
}

bool Packet::PacketFile::HasError()
{
	return m_ErrorObject.IsSet();
}

Packet::PacketFragment::FileIdentifier Packet::PacketFile::GetFileIdentifier()
{
	return m_FileIdentifier;
}

unsigned char* Packet::PacketFile::GetInternalDataPtr()
{
	return m_Data;
}

void Packet::PacketFile::FinishLoading()
{
	// Set ready
	m_IsReady = true;

	// Check if the callback was set
	if (m_LoadCallback)
	{
		// Call the load callback
		m_LoadCallback();
	}
}

unsigned char* Packet::PacketFile::AllocateMemory(uint32_t _fileSize)
{
	return new unsigned char[_fileSize];
}

void Packet::PacketFile::DeallocateMemory(unsigned char* _fileData)
{
	delete[] _fileData;
}

bool Packet::PacketFile::AllocateData(uint32_t _fileSize)
{
	// Check if our ptr is valid
	if(m_Data != nullptr)
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorFileDataAlreadyAllocated);
		return false;
	}

	// Call the virtual method for allocation
	m_Data = AllocateMemory(_fileSize);

	// Set ir dirty to false
	m_IsDirty = false;

	return true;
}