////////////////////////////////////////////////////////////////////////////////
// Filename: SmallPack.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include <cstdint>
#include <string>
#include <filesystem>
#include <functional>

/////////////
// DEFINES //
/////////////

#define PacketNamespaceBegin(name)					namespace name {
#define PacketNamespaceEnd(name)					}

#define PacketDevelopmentNamespaceBegin(name)		namespace __development__ ## name {
#define PacketDevelopmentNamespaceEnd(name)			}

#define PacketUsingNamespace(name)					using namespace name
#define PacketUsingDevelopmentNamespace(name)		using namespace __development__ ## name;

#define PacketDevelopmentNamespace(name)			__development__ ## name

PacketDevelopmentNamespaceBegin(Packet)

/////////////////////
// DEFINES / ENUMS //
/////////////////////

// The maximum file path name
static const int FilePathSize					= 128;

// The maximum package file size and the maximum number of files inside each package
static const uint64_t MaximumPackageSize		= 536870912;
static const uint32_t MaximumPackageFiles		= 2048;

// The reference file extension and the condensed file name/extension
static const std::string ReferenceExtension		= ".ref";
static const std::string CondensedName			= "Data";
static const std::string CondensedExtension		= ".pack";
static const std::string CondensedInfoName		= "Data";
static const std::string CondensedInfoExtension = ".manifest";
static const std::string TemporaryFileExtension = ".temp";

// The current condensed file minor and major versions
static const uint16_t CondensedMinorVersion		= 1;
static const uint16_t CondensedMajorVersion		= 0;

// The operation modes
enum class OperationMode
{
	Edit, 
	Condensed
};

// The reference fixer types
enum class ReferenceFixer
{
	None,
	MatchAll,
	AtLeastNameAndExtension,
	AtLeastName,
	NameAndExtensionOrSizeAndExtension
};

/////////////
// METHODS //
/////////////

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

// This method will return the root folder for the given path string
static std::string GetRootFolder(std::string _pathString)
{
	// Get the root folder name
	auto rootFolderPath = std::filesystem::path(_pathString);
	while (rootFolderPath.has_parent_path())
	{
		rootFolderPath = rootFolderPath.parent_path();
	}
	return std::filesystem::path(rootFolderPath).string().append("\\");
}

//////////////////////////
// CLASSES / STRUCTURES //
//////////////////////////

template <typename ResultT, ResultT OffsetBasis, ResultT Prime>
class basic_fnv1a final
{
	static_assert(std::is_unsigned<ResultT>::value, "need unsigned integer");

public:

	using result_type = ResultT;

private:

	result_type state_{};

public:

	constexpr
		basic_fnv1a() noexcept : state_{ OffsetBasis }
	{
	}

	constexpr void
		update(const char *const data, const std::size_t size) noexcept
	{
		auto acc = this->state_;
		for (auto i = std::size_t{}; i < size; ++i)
		{
			const auto next = std::size_t(data[i] );
			acc = (acc ^ next) * Prime;
		}
		this->state_ = acc;
	}

	constexpr void
		update(const char *const data) noexcept
	{
		auto acc = this->state_;
		for (auto i = std::size_t{}; data[i] != 0; ++i)
		{
			const auto next = std::size_t( data[i]);
			acc = (acc ^ next) * Prime;
		}
		this->state_ = acc;
	}

	constexpr result_type
		digest() const noexcept
	{
		return this->state_;
	}
};

using fnv1a_32 = basic_fnv1a<std::uint32_t,
	UINT32_C(2166136261),
	UINT32_C(16777619)>;

using fnv1a_64 = basic_fnv1a<std::uint64_t,
	UINT64_C(14695981039346656037),
	UINT64_C(1099511628211)>;

constexpr std::size_t fnv1a(const char *const _str, const std::size_t _size) noexcept
{
	fnv1a_32 hashfn;;
	hashfn.update(_str, _size);
	return hashfn.digest();
}

constexpr std::size_t fnv1a(const char *const _str) noexcept
{
	fnv1a_32 hashfn;;
	hashfn.update(_str);
	return hashfn.digest();
}

// The path type
struct Path
{
	Path() {}
	Path(char* _str)
	{
		strcpy_s(m_PathString, _str);
	}
	Path(std::string& _str)
	{
		strcpy_s(m_PathString, _str.c_str());
	}
	
	Path& operator =(const char* _str)
	{
		strcpy_s(m_PathString, _str);
		return *this;
	}
	Path& operator =(const std::string _str)
	{
		strcpy_s(m_PathString, _str.c_str());
		return *this;
	}

	operator const char*() const
	{
		return m_PathString;
	}

	const char* String() const
	{
		return m_PathString;
	}

	friend std::ostream& operator<< (std::ostream& _stream, const Path& _path) 
	{
		return _stream << _path.m_PathString;
	}

	bool Compare(const char* _str)
	{
		return strcmp(_str, m_PathString) == 0;
	}

	/*
	bool operator ==(const char* _str)
	{
		return strcmp(_str, m_PathString);
	}
	*/

private:

	char m_PathString[FilePathSize];
};

// The hash primitive type
typedef std::size_t HashPrimitive;

// The hash type
struct Hash
{
	Hash() {}
	Hash(const std::string _str)
	{
		m_Hash = fnv1a(_str.c_str());
		m_Path = _str;
	}
	Hash(const char* _str)
	{
		m_Hash = fnv1a(_str);
		m_Path = _str;
	}

	operator HashPrimitive()
	{
		return m_Hash;
	}

	bool operator ==(const Hash& b) const
	{
		return m_Hash == b.m_Hash;
	}

	// Comparator
	bool operator()(const Hash& a, const Hash& b) const
	{
		return a.m_Hash < b.m_Hash;
	}

	// Return the hash value
	const HashPrimitive GetHashValue() const
	{
		return m_Hash;
	}

	// Return the path reference
	const Path& GetPath()
	{
		return m_Path;
	}

private:

	// The hash properties
	HashPrimitive m_Hash;
	Path m_Path;
};

// The condensed header info
struct CondensedHeaderInfo
{
	// The time this file was saved
	uint64_t saveTime;

	// The total number of file infos
	uint32_t totalInfos;

	// The minor and major version indexes
	uint16_t minorVersion;
	uint16_t majorVersion;
};

// The packet condensed file info
struct CondensedFileInfo
{
	// A internal file info
	struct InternalFileInfo
	{
		// The internal file location, size, last updated time and hash
		Hash hash;
		uint64_t location;
		uint64_t size;
		uint64_t time;
	};

	// The path to the condensed file
	Path filePath;

	// The total number of files inside
	uint32_t totalNumberFiles = 0;

	// All the internal file infos
	InternalFileInfo fileInfos[MaximumPackageFiles];
};

// PacketFileLoader interface
class PacketFileLoader
{
public:

	// Constructor
	PacketFileLoader(std::string _packetManifestDirectory)
	{
		m_PacketFolderPath = _packetManifestDirectory;
	}

//////////////////
// MAIN METHODS //
public: //////////

	// Check if a given file exist
	virtual bool FileExist(Hash _fileHash) = 0;

	// Return a file size
	virtual uint64_t GetFileSize(Hash _fileHash) = 0;

	// Get the file data
	virtual bool GetFileData(uint8_t* _dataOut, uint64_t _bufferSize, Hash _fileHash) = 0;

	// Pack all files
	virtual bool ConstructPacket() = 0;

///////////////
// VARIABLES //
private: //////

protected:

	std::string m_PacketFolderPath;
};

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
class FutureReference
{
public:

	FutureReference()
	{
		// Set the initial data
		m_IsReady = false;
		m_InternalObject = nullptr;
	}

	// Set the internal object
	void SetInternalObject(ObjectType* _object)
	{
		// Set the object
		m_InternalObject = _object;

		// Set the boolean variable
		m_IsReady = true;
	}

	// Return if the internal object is ready
	// If this returns false, the internal object is still in process to be initialized, caution should be taken here
	bool IsRead()
	{
		return m_IsReady;
	}

	// Access the internal object
	ObjectType* operator->() const
	{
		return m_InternalObject;
	}

private:

	// If the internal object is ready
	bool m_IsReady;

	// The internal object
	ObjectType* m_InternalObject;
};

template <typename ObjectType>
ObjectType* GlobalInstance<ObjectType>::m_InternalObject = nullptr;

PacketDevelopmentNamespaceEnd(Packet)