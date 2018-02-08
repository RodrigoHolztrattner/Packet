////////////////////////////////////////////////////////////////////////////////
// Filename: PacketObjectIterator.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketFragment.h"
#include "PacketObjectManager.h"
#include "PacketObjectStructure.h"
#include "PacketObjectHashTable.h"
#include "PacketObjectIteratorPath.h"
#include "PacketObjectTemporaryPath.h"
#include "PacketError.h"

#include <string>
#include <stack>

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
// Class name: PacketObjectIterator
////////////////////////////////////////////////////////////////////////////////
class PacketObjectIterator
{
public:

//////////////////
// CONSTRUCTORS //
public: //////////

	// Constructor / destructor
	PacketObjectIterator(PacketObjectManager& _packetManagerReference, PacketObjectStructure& _packetStructureManager, PacketObjectHashTable& _packetHashTableReference);
	~PacketObjectIterator();

//////////////////
// MAIN METHODS //
public: //////////

	/**
	 * @fn	bool PacketObjectIterator::Seek(std::string _path);
	 *
	 * @summary	Seeks the given path.
	 *
	 * @author	Zapdos
	 * @date	2/8/2018
	 *
	 * @param	_path	Full pathname of the file.
	 *
	 * @return	True if it succeeds, false if it fails.
	 *
	 * ### remarks	Zapdos, 2/8/2018.
	 */
	bool Seek(std::string _path);

	/**
	 * @fn	bool PacketObjectIterator::MakeDir(std::string _dirPath);
	 *
	 * @summary	Makes a dir.
	 *
	 * @author	Zapdos
	 * @date	2/8/2018
	 *
	 * @param	_dirPath	Pathname of the directory.
	 *
	 * @return	True if it succeeds, false if it fails.
	 *
	 * ### remarks	Zapdos, 2/8/2018.
	 */
	bool MakeDir(std::string _dirPath);

	/**
	 * @fn	bool PacketObjectIterator::Put(unsigned char* _data, uint32_t _size);
	 *
	 * @summary	Put a file/data <inside the current path>.
	 *
	 * @author	Zapdos
	 * @date	2/8/2018
	 *
	 * @param			_data	If non-null, the data.
	 * @param 		  	_size	The size.
	 * @param 		 	_iFolderLocation	Where the data should be located internally.
	 * @param 		 	_filePath			External file location.
	 * 																			
	 * @return	True if it succeeds, false if it fails.
	 *
	 * ### remarks	Zapdos, 2/8/2018.
	 */
	bool Put(unsigned char* _data, uint32_t _size);
	bool Put(unsigned char* _data, uint32_t _size, std::string _iFolderLocation);
	bool Put(std::string _filePath, std::string _iFolderLocation);
	bool Put(std::string _filePath);

	/**
	 * @fn	bool PacketObjectIterator::Get(std::string _iFileLocation, unsigned char* _data);
	 *
	 * @summary	Get a file/data -from the current path-.
	 *
	 * @author	Zapdos
	 * @date	2/8/2018
	 *
	 * @param 	   	_iFileLocation	The internal file location.
	 * @param [out]	_data		  	If non-null, the data.
	 * @param		_oFileLocation	Where (externally) the data should be located (file).				
	 *
	 * @return	True if it succeeds, false if it fails.
	 *
	 * ### remarks	Zapdos, 2/8/2018.
	 */
	bool Get(std::string _iFileLocation, unsigned char* _data);
	bool Get(std::string _iFileLocation);								
	bool Get(std::string _iFileLocation, std::string _oFileLocation);	

	/**
	 * @fn	bool PacketObjectIterator::Delete(std::string _iLocation);
	 *
	 * @brief	Deletes the current path <_iLocation>.
	 *
	 * @author	Zapdos
	 * @date	2/8/2018
	 *
	 * @param	_iLocation	The location to delete.
	 *
	 * @return	True if it succeeds, false if it fails.
	 */
	bool Delete(std::string _iLocation);

	/**
	 * @fn	bool PacketObjectIterator::Optimize();
	 *
	 * @brief	Optimizes the internal fragments.
	 *
	 * @author	Zapdos
	 * @date	2/8/2018
	 *
	 * @return	True if it succeeds, false if it fails.
	 */
	bool Optimize();

	/**
	 * @fn	std::vector<std::string> PacketObjectIterator::List();
	 *
	 * @brief	Get a list of each folder and file from the current path or from the input path. 
	 *
	 * @author	Zapdos
	 * @date	2/8/2018
	 * 			
	 * @param	_path	The location.
	 *
	 * @return	A std::vector&lt;std::string&gt;
	 */
	std::vector<std::string> List();
	std::vector<std::string> List(std::string _path);

	/**
	 * @fn	std::string PacketObjectIterator::GetCurrentPath();
	 *
	 * @brief	Return the current path. 
	 *
	 * @author	Zapdos
	 * @date	2/8/2018
	 *
	 * @return	The current path.
	 */
	std::string GetCurrentPath();

	/**
	 * @fn	PacketError PacketObjectIterator::GetError();
	 *
	 * @brief	Return the error object. 
	 *
	 * @author	Zapdos
	 * @date	2/8/2018
	 *
	 * @return	The error.
	 */
	PacketError GetError();

private:

	// Put a file <inside the given path> aux
	bool PutAux(PacketObjectTemporaryPath& _temporaryPath);

	// Get a file <from the given path> aux
	bool GetAux(PacketObjectTemporaryPath& _temporaryPath);

	// Delete a file/folder recursivelly
	bool DeleteFile(std::string _iFileLocation);
	bool DeleteFolder(std::string _iFolderLocation);

private:

///////////////
// VARIABLES //
private: //////

	// The iterator path
	PacketObjectIteratorPath m_IteratorPath;

	// The packet object manager reference
	PacketObjectManager& m_PacketManagerReference;

	// The packet structure reference
	PacketObjectStructure& m_PacketStructureReference;

	// The packet hash table reference
	PacketObjectHashTable& m_PacketHashTableReference;

	// The error object
	PacketError m_ErrorObject;
};

// Packet data explorer
PacketNamespaceEnd(Packet)
