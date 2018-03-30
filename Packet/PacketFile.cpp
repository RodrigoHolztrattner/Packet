////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFile.h"
#include "PacketObject.h"
#include "PacketFileLoader.h"
#include "PacketFileRemover.h"
#include "PacketFileReference.h"

Packet::PacketFile::PacketFile()
{
	// Set our initial data
	m_IsReady = false;
	m_IsDirty = true;
	m_Data = nullptr;
}

Packet::PacketFile::~PacketFile()
{
	// Check if we should deallocate our data
	if(m_Data != nullptr)
	{
		OnMemoryDeallocationRequest(m_Data);
		m_Data = nullptr;
	}
}

Packet::PacketFile::DispatchType Packet::PacketFile::GetDispatchType()
{
	return m_DispatchType;
}

void Packet::PacketFile::Release()
{
	// Call the OnRelease() callback virtual method
	OnRelease();

	// Deallocate the memory
	m_Data = nullptr;
	OnMemoryDeallocationRequest(m_Data);
}

uint32_t Packet::PacketFile::GetReferenceCount()
{
	return m_TotalNumberReferences;
}

void Packet::PacketFile::IncrementReferenceCount()
{
	m_TotalNumberReferences++;
}

void Packet::PacketFile::DecrementReferenceCount()
{
	m_TotalNumberReferences--;
}

bool Packet::PacketFile::WasLoaded()
{
	return m_IsReady && !m_IsDirty;
}

bool Packet::PacketFile::AllocationIsDelayed()
{
	return m_DelayAllocation;
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
	// Lock our reference mutex
	m_ReferenceMutex.lock();

	// Set ready
	m_IsReady = true;

	// Unlock our reference mutex
	m_ReferenceMutex.unlock();

	// Call the callback method
	OnLoadingCompleted();

	// For each reference request
	for (auto& referenceRequest : m_FileReferenceRequests)
	{
		// Call the ready callback for this reference
		referenceRequest->CallReadyCallback();
	}

	// Clear the reference vector
	m_FileReferenceRequests.clear();
}

void Packet::PacketFile::AddFileReferenceRequest(PacketFileReference* _fileReferenceRequest)
{
	// Lock our reference mutex
	std::lock_guard<std::mutex> guard(m_ReferenceMutex);

	// Check if this file is ready
	if (m_IsReady)
	{
		// Call the ready callback
		_fileReferenceRequest->CallReadyCallback();
	}
	else
	{
		// Add this reference to our request vector
		m_FileReferenceRequests.push_back(_fileReferenceRequest);
	}
}

unsigned char* Packet::PacketFile::OnMemoryAllocationRequest(uint32_t _fileSize)
{
	return new unsigned char[_fileSize];
}

void Packet::PacketFile::OnMemoryDeallocationRequest(unsigned char* _fileData)
{
	delete[] _fileData;
}

void Packet::PacketFile::SetLoadParams(PacketFragment::FileIdentifier _fileIdentifier, DispatchType _dispatchType, bool _delayAllocation)
{
	m_FileIdentifier = _fileIdentifier;
	m_DispatchType = _dispatchType;
	m_DelayAllocation = _delayAllocation;
}

bool Packet::PacketFile::AllocateData(uint32_t _fileSize)
{
	// Check if our ptr is valid
	if(m_Data != nullptr)
	{
		// Set the error
		return false;
	}

	// Call the virtual method for allocation
	m_Data = OnMemoryAllocationRequest(_fileSize);

	// Set ir dirty to false
	m_IsDirty = false;

	return true;
}