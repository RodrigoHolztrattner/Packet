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
		_position += sizeof(ObjectSize);
		return ObjectType(_data[_position - sizeof(ObjectSize)]);
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
	static std::string ReadFromData(std::vector<unsigned char>& _data, uint32_t& _position, uint32_t _amount)
	{
		std::string result(_data.begin(), _data.end());
		_position += sizeof(unsigned char) * _amount;
		return result;
	}

	// Save to data
	template<typename ObjectType>
	static void SaveToData(unsigned char* _data, uint32_t& _position, ObjectType _object)
	{
		memcpy(&_data[_position], &_object, sizeof(ObjectType));
		_position += sizeof(ObjectType);
	}

	// Save to data
	template<typename ObjectType>
	static void SaveToData(unsigned char* _data, uint32_t& _position, ObjectType _object, uint32_t _amount)
	{
		memcpy(&_data[_position], &_object, sizeof(ObjectType) * _amount);
		_position += sizeof(ObjectType) * _amount;
	}

	// Save to data
	static void SaveToData(std::vector<unsigned char>& _data, uint32_t& _position, std::string& _string)
	{
		std::vector<unsigned char> temp(_string.begin(), _string.end());
		_data.insert(_data.end(), temp.begin(), temp.end());
		_position += sizeof(unsigned char) * _string.size();
	}

///////////////
// VARIABLES //
private: //////

};

// Packet data explorer
PacketNamespaceEnd(Packet)
