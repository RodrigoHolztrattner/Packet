////////////////////////////////////////////////////////////////////////////////
// Filename: FluxMyWrapper.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketString.h"

Packet::PacketString::PacketString()
{
	// Set the initial data
	// ...
}

Packet::PacketString::PacketString(const char* _string)
{
	if (strlen(_string) <= MaxStringSize)
		strcpy(m_String, _string);
}

Packet::PacketString::~PacketString()
{
}

void Packet::PacketString::operator =(const char* _string)
{
	if (strlen(_string) <= MaxStringSize)
		strcpy(m_String, _string);
}

void Packet::PacketString::SetString(const char* _string)
{
	if (strlen(_string) <= MaxStringSize)
		strcpy(m_String, _string);
}

void Packet::PacketString::SetString(const char* _string, uint32_t _stringSize)
{
	if ((_stringSize + 1) <= MaxStringSize) // Include the null terminator
	{
		// Copy the string
		memcpy(m_String, _string, sizeof(char) * (_stringSize));

		// Include the null terminator
		m_String[_stringSize] = 0;
	}
}

char* Packet::PacketString::GetString()
{
	return m_String;
}

bool Packet::PacketString::IsEqual(PacketString& _string, uint32_t _stringSize)
{
	return IsEqual(_string.GetString(), _stringSize);
}

bool Packet::PacketString::IsEqual(const char* _string, uint32_t _stringSize)
{
	uint32_t currentSize = 0;
	while (currentSize < _stringSize)
	{
		if (m_String[currentSize] != _string[currentSize])
		{
			return false;
		}

		currentSize++;
	}

	return true;
}