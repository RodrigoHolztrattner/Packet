////////////////////////////////////////////////////////////////////////////////
// Filename: PacketLoader.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include <cstdint>

///////////////
// NAMESPACE //
///////////////

/////////////
// DEFINES //
/////////////

////////////
// GLOBAL //
////////////

///////////////
// NAMESPACE //
///////////////

#define PacketNamespaceBegin(name) namespace name {
#define PacketNamespaceEnd(name) }

/*

=> Pre-processor constant hashed strings only are created if those flags..:

- C/C++ > Optimization > Full Optimization
- C/C++ > Code Generation > Enable String Pooling

are set to true... So, if you wanna use this, remember to change the project properties.

*/

PacketNamespaceBegin(Packet)

typedef uint32_t HashedStringIdentifier;

HashedStringIdentifier constexpr SimpleHash(char const *input)
{
	return *input ?
		static_cast<uint32_t>(*input) + 33 * SimpleHash(input + 1) :
		5381;
}

////////////////////////////////////////////////////////////////////////////////
// Class name: HashedString
////////////////////////////////////////////////////////////////////////////////
class HashedString
{
public:

	HashedString() {}
	HashedString(const char* _string)
	{
		m_String = _string;
		m_Hash = SimpleHash(_string);
	}

	// Compare 2 hashed strings
	bool operator() (HashedString const& t1, HashedString const& t2) const
	{
		return (t1.m_Hash == t2.m_Hash);
	}

public:

	// Return the original string
	const char* String()
	{
		return m_String;
	}

	// Return the hash
	HashedStringIdentifier Hash()
	{
		return m_Hash;
	}

public:

	// The hash object (pre-calculated if we use the full optimization flag)
	HashedStringIdentifier m_Hash;

	// The original string
	const char* m_String;
};

PacketNamespaceEnd(Packet)