////////////////////////////////////////////////////////////////////////////////
// Filename: PacketFileDataOperations.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"

#include <string>
#include <vector>

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

// Packet data explorer
PacketNamespaceBegin(Packet)

////////////////
// FORWARDING //
////////////////

////////////////
// STRUCTURES //
////////////////

////////////////////////////////////////////////////////////////////////////////
// Class name: PacketFileDataOperations
////////////////////////////////////////////////////////////////////////////////
class PacketFileDataOperations
{
private:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketFileDataOperations();
	~PacketFileDataOperations();

//////////////////
// MAIN METHODS //
public: //////////

	// Read from data
	template<typename ObjectType>
	static ObjectType ReadFromData(std::vector<unsigned char>& _data, uint32_t& _position)
	{
		_position += sizeof(ObjectType);
		unsigned char* data = &_data[_position - sizeof(ObjectType)];
		return *(ObjectType*)(data);
	}

	// Read from data
	template<typename ObjectType>
	static std::vector<ObjectType> ReadFromData(std::vector<unsigned char>& _data, uint32_t& _position, uint32_t _amount)
	{
		std::vector<ObjectType> result(&_data.data()[_position], &_data.data()[_position] + sizeof(ObjectType) * _amount);
		_position += sizeof(ObjectType) * _amount;
		return result;
	}

	// Read from data
	static std::string ReadFromData(std::vector<unsigned char>& _data, uint32_t& _position)
	{
		// Read the string size from the data
		uint32_t stringSize = ReadFromData<uint32_t>(_data, _position);

		// Read the result string
		std::string result(_data.begin() + _position, _data.begin() + _position + stringSize);

		// Update the position
		_position += sizeof(unsigned char) * stringSize;

		return result;
	}

	// Save to data
	template<typename ObjectType>
	static void SaveToData(std::vector<unsigned char>& _data, uint32_t& _position, ObjectType _object)
	{
		InsertDataIntoVector(_data, &_object, 1);
		_position += sizeof(ObjectType);
	}

	// Save to data
	template<typename ObjectType>
	static void SaveToData(unsigned char* _data, uint32_t& _position, ObjectType* _object, uint32_t _amount)
	{
		InsertDataIntoVector(_data, _object, _amount);
		_position += sizeof(ObjectType) * _amount;
	}

	// Save to data
	static void SaveToData(std::vector<unsigned char>& _data, uint32_t& _position, std::string& _string)
	{
		// The string size
		uint32_t stringSize = (uint32_t)_string.size();

		// Save the string size from the data
		SaveToData<uint32_t>(_data, _position, stringSize);

		// Copy the string data
		std::vector<unsigned char> temp(_string.begin(), _string.end());
		_data.insert(_data.end(), temp.begin(), temp.end());
		_position += sizeof(unsigned char) * (uint32_t)_string.size();
	}

private:

	// Insert data into a vector
	template<typename ObjectType>
	static void InsertDataIntoVector(std::vector<unsigned char>& _vector, ObjectType* _object, uint32_t _amount)
	{
		unsigned char* values = (unsigned char*)_object;
		unsigned char* end = values + sizeof(ObjectType) * _amount;

		_vector.insert(_vector.end(), values, end);
	}

///////////////
// VARIABLES //
private: //////

};

// Packet data explorer
PacketNamespaceEnd(Packet)
