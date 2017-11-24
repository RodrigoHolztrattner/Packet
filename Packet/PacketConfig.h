////////////////////////////////////////////////////////////////////////////////
// Filename: SmallPack.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////

/////////////
// DEFINES //
/////////////

#define PacketNamespaceBegin(name) namespace name {
#define PacketNamespaceEnd(name) }

#define PacketUsingNamespace(name) using namespace name

PacketNamespaceBegin(Packet)

inline unsigned int SetFlag(unsigned int _val, unsigned int _flag) { return _val | _flag; }
inline bool CheckFlag(unsigned int _val, unsigned int _flag) { return (_val & _flag) != 0; }

inline unsigned int pow2roundup(unsigned int x)
{
	if (x < 0)
		return 0;
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}

template <typename ObjectType>
struct GlobalInstance
{
	// Constructor
	GlobalInstance()
	{
		// Create a new instance in case of nullptr
		if (m_InternalObject == nullptr)
		{
			m_InternalObject = new ObjectType();
		}
	}

	// Return the instance
	const static ObjectType* GetInstance()
	{
		return m_InternalObject;
	}

	// Access the internal object
	GlobalInstance operator=(ObjectType* _other)
	{
		m_InternalObject = _other;
		return *this;
	}

	operator ObjectType*()
	{
		return m_InternalObject;
	}

	// Access the internal object
	ObjectType* operator->() const
	{
		//m_iterator is a map<int, MyClass>::iterator in my code.
		return m_InternalObject;
	}

private:

	// The internal object
	static ObjectType* m_InternalObject;
};

template <typename ObjectType>
ObjectType* GlobalInstance<ObjectType>::m_InternalObject = nullptr;

PacketNamespaceEnd(Packet)