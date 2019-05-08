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
#include <iostream>
#include <nlohmann/json.hpp>

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

// The maximum file path name size (including the null terminated char, max length 119)
static const int FilePathSize					= 120;

// The maximum file type name size
static const int FileTypeSize                   = 32;

// The icon image length and total size
static const int IconLengthSize                 = 64;
static const int IconTotalSize                  = IconLengthSize * IconLengthSize * 4;

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
			acc = ResultT((acc ^ next) * Prime);
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
			acc = ResultT((acc ^ next) * Prime);
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

// The logger class
struct PacketLogger
{
	virtual void LogError(const char* _str)
	{
		std::cout << "Packet Error: " << _str << std::endl;
	}

	virtual void LogWarning(const char* _str)
	{
		std::cout << "Packet Warning: " << _str << std::endl;
	}

	virtual void LogInfo(const char* _str)
	{
		std::cout << "Packet Info: " << _str << std::endl;
	}

	virtual void Log(const char* _str)
	{
		std::cout << _str << std::endl;
	}

	virtual ~PacketLogger() {}
};

// The resource build info
struct PacketResourceBuildInfo
{
	PacketResourceBuildInfo() {}
	PacketResourceBuildInfo(uint32_t _buildFlags) : 
		buildFlags(_buildFlags) {}
	PacketResourceBuildInfo(uint32_t _buildFlags, uint32_t _flags) : 
		buildFlags(_buildFlags), 
		flags(_flags) {}
	PacketResourceBuildInfo(uint32_t _buildFlags, uint32_t _flags, bool _asyncInstanceConstruct) :
		buildFlags(_buildFlags),
		flags(_flags),
		asyncInstanceConstruct(_asyncInstanceConstruct) {}
	PacketResourceBuildInfo(uint32_t _buildFlags, uint32_t _flags, bool _asyncInstanceConstruct, bool _asyncResourceObjectDeletion) :
		buildFlags(_buildFlags), 
		flags(_flags), 
		asyncInstanceConstruct(_asyncInstanceConstruct),
		asyncResourceObjectDeletion(_asyncResourceObjectDeletion) {}

	// The build flags (resource with different build flags and equal hash objects are considered different between each other)
	uint32_t buildFlags = 0;

	// The normal flags (resource with different normal flags and equal hash objects are considered equal between each other)
	uint32_t flags = 0;

	// If the Instance Object can have its OnConstruct() method called asynchronous (without being sure if that will 
	// happen inside the update phase of the resource manager)
	bool asyncInstanceConstruct = true;

	// If the Resource Object can be deleted synchronous (without being sure if that will happen inside the update 
	// phase of the resource manager)
	bool asyncResourceObjectDeletion = true;
};

// The path type
template <uint32_t TotalSize>
struct FixedSizeString
{
    FixedSizeString() {}
    FixedSizeString(char* _str)
	{
        std::string temp(_str);
        std::copy(temp.begin(), temp.end(), m_PathString.data());
    }
    FixedSizeString(std::string& _str)
	{
        std::copy(_str.begin(), _str.end(), m_PathString.data());
    }
	
    FixedSizeString& operator =(const char* _str)
	{
        std::string temp(_str);
        std::copy(temp.begin(), temp.end(), m_PathString.data());
        return *this;
	}
    FixedSizeString& operator =(const std::string _str)
	{
        std::copy(_str.begin(), _str.end(), m_PathString.data());
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

	friend std::ostream& operator<< (std::ostream& _stream, const FixedSizeString& _path)
	{
		return _stream << _path.m_PathString;
	}

	bool Compare(const char* _str)
	{
		return strcmp(_str, m_PathString) == 0;
	}

    const std::array<char, TotalSize>& GetRaw() const
    {
        return m_PathString;
    }

    std::array<char, TotalSize>& GetRaw()
    {
        return m_PathString;
    }

private:

	std::array<char, TotalSize> m_PathString;
};

namespace ns {
    template <uint32_t TotalSize>
    void to_json(nlohmann::json& j, const FixedSizeString<TotalSize>& s) {
        
        j = nlohmann::json{ {"Path", s.GetRaw()}};
    }

    template <uint32_t TotalSize>
    void from_json(const nlohmann::json& j, FixedSizeString<TotalSize>& s) {
        j.at("Path").get_to(s.GetRaw());
    }
} // namespace ns

typedef FixedSizeString<FilePathSize> Path;
typedef FixedSizeString<FileTypeSize> FileType;
typedef uint64_t FileDataPosition;
typedef uint64_t FileDataSize;

namespace ns {
    void to_json(nlohmann::json& j, const Path& s) {

        j = nlohmann::json{ {"Path", s.GetRaw()} };
    }

    void from_json(const nlohmann::json& j, Path& s) {
        j.at("Path").get_to(s.GetRaw());
    }
} // namespace ns

// The hash primitive type
typedef uint64_t HashPrimitive;

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

    bool operator< (const Hash& a) const
    {
        return a.m_Hash < this->m_Hash;
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
    Path m_Path;
	HashPrimitive m_Hash;
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
	PacketFileLoader(std::string _packetManifestDirectory, PacketLogger* _logger) : m_PacketFolderPath(_packetManifestDirectory), m_Logger(_logger) {}

//////////////////
// MAIN METHODS //
public: //////////

	// Check if a given file exist
	virtual bool FileExist(Hash _fileHash) const = 0;

	// Return a file size
	virtual uint64_t GetFileSize(Hash _fileHash) const = 0;

	// Get the file data
	virtual bool GetFileData(uint8_t* _dataOut, uint64_t _bufferSize, Hash _fileHash) const = 0;

	// Pack all files
	virtual bool ConstructPacket() = 0;

///////////////
// VARIABLES //
protected: ////

	std::string m_PacketFolderPath;
	PacketLogger* m_Logger;
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