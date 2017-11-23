////////////////////////////////////////////////////////////////////////////////
// Filename: GlobalInstance.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////
// LINKING //
/////////////

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include <vector>

///////////////
// NAMESPACE //
///////////////

// Packet Serializer
PacketNamespaceBegin(Packet)
PacketNamespaceBegin(Serialize)

/////////////
// DEFINES //
/////////////

class Serializable
{
public:

	// Serialize this object and append
	virtual void SerializeAndAppend(std::vector<unsigned char>& _byteArray)
	{
		std::vector<unsigned char> serializedData = Serialize();
		_byteArray.insert(_byteArray.end(), serializedData.begin(), serializedData.end());
	}

	// Serialize this object
	virtual std::vector<unsigned char> Serialize() = 0;

	// Deserialize this
	virtual uint32_t Deserialize(std::vector<unsigned char>& _data, uint32_t _index) = 0;
};

class Serializer
{
public:

	Serializer(std::vector< unsigned char >& _dst) : m_Dst(_dst) {}

	template <typename T>
	inline void PackData(T& data)
	{
		unsigned char * src = reinterpret_cast < unsigned char* >(&data); // 	unsigned char * src = static_cast < unsigned char* >(static_cast < void * >(&data));
		m_Dst.insert(m_Dst.end(), src, src + sizeof(T));
	}

	inline void PackData(char* data, uint32_t total)
	{
		unsigned char * src = reinterpret_cast < unsigned char* >(data);
		m_Dst.insert(m_Dst.end(), src, src + sizeof(char) * total);
	}

	template <typename T>
	inline void PackData(T* data, uint32_t total)
	{
		PackData((char*)data, sizeof(T) * total);
	}

private:

	std::vector< unsigned char >& m_Dst;
};

class Deserializer
{
public:

	Deserializer(std::vector< unsigned char >& _src, uint32_t _index) : m_Src(_src) { m_Index = _index; }

	template <typename T>
	inline void UnpackData(T& data)
	{
		memcpy(&data, &m_Src.data()[m_Index], sizeof(T));
		m_Index += sizeof(T);
	}

	template <typename T>
	inline void UnpackData(T* data, uint32_t total)
	{
		memcpy(data, &m_Src.data()[m_Index], sizeof(T) * total);
		m_Index += sizeof(T) * total;
	}

	uint32_t GetIndex() { return m_Index; }

private:

	std::vector< unsigned char >& m_Src;
	uint32_t m_Index;
};

// Packet Serializer
PacketNamespaceEnd(Serialize)
PacketNamespaceEnd(Packet)