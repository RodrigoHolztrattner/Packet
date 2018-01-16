////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketFile.h"
#include "PacketObject.h"
#include "PacketFileLoader.h"

Packet::PacketFile::PacketFile(PacketObject* _packetObject, DispatchType _dispatchType, bool _delayAllocation)
{
	// Set our initial data
	m_PacketObjectReference = _packetObject;
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

bool Packet::PacketFile::LoadWithIdentifier(PacketFragment::FileIdentifier _fileIdentifier)
{
	// Get the packet file loader instance
	PacketFileLoader* packetFileLoader = m_PacketObjectReference->GetFileLoader();

	// Set the metadata
	m_Metadata.fileIdentifier = _fileIdentifier;
	if (!packetFileLoader->GetFileData(_fileIdentifier, m_Metadata.fileSize, m_Metadata.fileFragmentIdentifier))
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorInvalidFileIdentifier);
		return false;
	}

	// Set dirty to false (we should use a barrier here to prevent random ordering?)
	m_IsDirty = false;

	// Process this packet file
	return packetFileLoader->ProcessPacketFile(this);
}

bool Packet::PacketFile::LoadWithName(const char* _fileName)
{
	return LoadWithIdentifier(HashFilePathStatic(_fileName));
}

void Packet::PacketFile::SetLoadCallback(std::function<void()> _loadCallback)
{
	m_LoadCallback = _loadCallback;
}

Packet::PacketFile::Metadata Packet::PacketFile::GetMetadata()
{
	return m_Metadata;
}

Packet::PacketFile::DispatchType Packet::PacketFile::GetDispatchType()
{
	return m_DispatchType;
}

bool Packet::PacketFile::IsReady()
{
	return m_IsReady;
}

bool Packet::PacketFile::AllocationIsDelayed()
{
	return m_DelayAllocation;
}

bool Packet::PacketFile::IsDirty()
{
	return m_IsDirty;
}

Packet::PacketFragment::FileIdentifier Packet::PacketFile::GetFileIdentifier()
{
	return m_Metadata.fileIdentifier;
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

unsigned char* Packet::PacketFile::AllocateMemory(unsigned int _fileSize)
{
	return new unsigned char[_fileSize];
}

void Packet::PacketFile::DeallocateMemory(unsigned char* _fileData)
{
	delete[] _fileData;
}

bool Packet::PacketFile::AllocateData()
{
	// Check if our ptr is valid
	if(m_Data != nullptr)
	{
		// Set the error
		m_ErrorObject.Set(PacketErrorFileDataAlreadyAllocated);
		return false;
	}

	// Call the virtual method for allocation
	m_Data = AllocateMemory(m_Metadata.fileSize);

	return true;
}