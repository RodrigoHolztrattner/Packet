////////////////////////////////////////////////////////////////////////////////
// Filename: SmallPack.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include <cstdint>
#include <string>
#include <optional>
#include <filesystem>
#include <functional>
#include <iostream>
#include <fstream>
#include <set>
#include <ctti/type_id.hpp>
#include <ctti/static_value.hpp>
#include <nlohmann/json.hpp>

/////////////
// DEFINES //
/////////////

#define PacketNamespaceBegin(name)                  namespace name {
#define PacketNamespaceEnd(name)                    }

#define PacketDevelopmentNamespaceBegin(name)       namespace __development__ ## name {
#define PacketDevelopmentNamespaceEnd(name)         }

#define PacketUsingNamespace(name)                  using namespace name
#define PacketUsingDevelopmentNamespace(name)       using namespace __development__ ## name;

#define PacketDevelopmentNamespace(name)            __development__ ## name

PacketDevelopmentNamespaceBegin(Packet)

/////////////////////
// DEFINES / ENUMS //
/////////////////////

// Our current version
static const uint32_t PacketVersion                 = 0;

// The file magic, used to verify the initial integrity
static const uint32_t FileMagic                     = 699555;

// The maximum file path name size (including the null terminated char, max length 119)
static const int FilePathSize                       = 120;

// The maximum file type name size
static const int FileTypeSize                       = 32;

// The icon image length and total size
static const int IconLengthSize                     = 64;
static const int IconTotalSize                      = IconLengthSize * IconLengthSize * 4;

// The maximum package file size and the maximum number of files inside each package
static const uint64_t MaximumPackageSize            = 536870912;
static const uint32_t MaximumPackageFiles           = 2048;

// The reference file extension and the condensed file name/extension
static const std::string CondensedName              = "Data";
static const std::string CondensedExtension         = ".pack";
static const std::string CondensedInfoName          = "Data";
static const std::string CondensedInfoExtension     = ".manifest";
static const std::string TemporaryFileExtension     = ".temp";
static const std::string IndexerCacheData           = "IndexCache.tmp";
static const std::string DefaultConverterExtension  = ".pktdefault";

// Default separator
static const char ForwardSlashSeparator             = '/';
static const char BackSlashSeparator                = '\\';
static const char DefaultSeparator                  = ForwardSlashSeparator;

// The current condensed file minor and major versions
static const uint16_t CondensedMinorVersion         = 1;
static const uint16_t CondensedMajorVersion         = 0;

// Bitfield, modified version from https://github.com/GPUOpen-LibrariesAndSDKs/Anvil/blob/master/include/misc/types_enums.h
template<typename IndividualBitEnumType, typename FlagsType = uint32_t>
class Bitfield
{
public:
    Bitfield()
        :m_value(static_cast<FlagsType>(0))
    {
    }

    Bitfield(const IndividualBitEnumType& in_bit)
        :m_value(static_cast<FlagsType>(in_bit))
    {
    }

    Bitfield(const Bitfield<IndividualBitEnumType, FlagsType>& in_bits)
        :m_value(in_bits.m_value)
    {
    }

    inline Bitfield<IndividualBitEnumType, FlagsType> operator&(const IndividualBitEnumType& in_bit) const
    {
        auto result = Bitfield<IndividualBitEnumType, FlagsType>(static_cast<IndividualBitEnumType>(m_value & static_cast<FlagsType>(in_bit)));

        return result;
    }

    inline Bitfield<IndividualBitEnumType, FlagsType> operator&(const IndividualBitEnumType& in_bit)
    {
        auto result = Bitfield<IndividualBitEnumType, FlagsType>(static_cast<IndividualBitEnumType>(m_value & static_cast<FlagsType>(in_bit)));

        return result;
    }

    inline Bitfield<IndividualBitEnumType, FlagsType>& operator|=(const IndividualBitEnumType& in_bit)
    {
        m_value |= static_cast<FlagsType>(in_bit);

        return *this;
    }

    inline Bitfield<IndividualBitEnumType, FlagsType>& operator|=(const Bitfield<IndividualBitEnumType, FlagsType>& in_bits)
    {
        m_value |= static_cast<FlagsType>(in_bits.m_value);

        return *this;
    }

    inline Bitfield<IndividualBitEnumType, FlagsType>& operator&=(const IndividualBitEnumType& in_bit)
    {
        m_value &= static_cast<FlagsType>(in_bit);

        return *this;
    }

    inline Bitfield<IndividualBitEnumType, FlagsType>& operator&=(const Bitfield<IndividualBitEnumType, FlagsType>& in_bits)
    {
        m_value &= static_cast<FlagsType>(in_bits.m_value);

        return *this;
    }

    inline Bitfield<IndividualBitEnumType, FlagsType>& operator=(const IndividualBitEnumType& in_bit)
    {
        m_value = static_cast<FlagsType>(in_bit);

        return *this;
    }

    inline Bitfield<IndividualBitEnumType, FlagsType>& operator=(const Bitfield<IndividualBitEnumType, FlagsType>& in_bits)
    {
        m_value = static_cast<FlagsType>(in_bits.m_value);

        return *this;
    }

    inline operator bool() const
    {
        return m_value != 0;
    }

    inline bool operator!=(const int& in_value) const
    {
        return m_value != static_cast<FlagsType>(in_value);
    }

    inline bool operator!=(const Bitfield<IndividualBitEnumType, FlagsType>& in_bitfield) const
    {
        return (m_value != in_bitfield.m_value);
    }

    inline bool operator!=(const IndividualBitEnumType& in_bit) const
    {
        return (m_value != static_cast<FlagsType>(in_bit));
    }

    inline bool operator==(const int& in_value) const
    {
        return m_value == static_cast<FlagsType>(in_value);
    }

    inline bool operator==(const Bitfield<IndividualBitEnumType, FlagsType>& in_bitfield) const
    {
        return (m_value == in_bitfield.m_value);
    }

    inline bool operator==(const IndividualBitEnumType& in_bit) const
    {
        return (m_value == static_cast<FlagsType>(in_bit));
    }

    inline bool operator<(const Bitfield<IndividualBitEnumType, FlagsType>& in_bitfield) const
    {
        return (m_value < in_bitfield.m_value);
    }

    inline bool operator<(const IndividualBitEnumType& in_bit) const
    {
        return (m_value < static_cast<FlagsType>(in_bit));
    }

    inline bool operator<=(const Bitfield<IndividualBitEnumType, FlagsType>& in_bitfield) const
    {
        return (m_value <= in_bitfield.m_value);
    }

    inline bool operator<=(const IndividualBitEnumType& in_bit) const
    {
        return (m_value <= static_cast<FlagsType>(in_bit));
    }

    inline bool operator>=(const Bitfield<IndividualBitEnumType, FlagsType>& in_bitfield) const
    {
        return (m_value >= in_bitfield.m_value);
    }

    inline bool operator>=(const IndividualBitEnumType& in_bit) const
    {
        return (m_value >= static_cast<FlagsType>(in_bit));
    }

    inline const FlagsType& get_raw() const
    {
        return m_value;
    }

    inline const FlagsType* get_raw_ptr() const
    {
        return &m_value;
    }

private:
    FlagsType m_value;
};

// The operation modes
enum class OperationMode
{
    Plain, 
    Condensed
};

// The file parts
enum class FilePart
{
    Header,
    IconData,
    PropertiesData,
    OriginalData,
    IntermediateData,
    FinalData,
    ReferencesData
};

// Import file flags
enum class FileImportFlagBits
{
    None,
    Overwrite
};

// Write file flags
enum class FileWriteFlagBits
{
    None                      = 0,
    Overwrite                 = 1 << 0,
    IgnoreMissingDependencies = 1 << 1
};

// The save operation that must be passed to the file saver when saving a file into disk
enum class SaveOperation
{
    Create, 
    Overwrite,
    Move,
    Copy,
    Rename, 
    ReferenceUpdate
};

// Our supported backup flags for the Plain operation mode
enum BackupFlags
{
    None                                   = 0, 
    BackupBeforeOperation                  = 1 << 0, 
    BackupOnStartup                        = 1 << 1, 
    AutomaticallyRestoreOnOperationFailure = 1 << 2
};

typedef Bitfield<FileImportFlagBits> FileImportFlags;
typedef Bitfield<FileWriteFlagBits>  FileWriteFlags;

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
		apply(const char *const data, const std::size_t size) noexcept
	{
		auto acc = this->state_;
		for (auto i = std::size_t{}; i < size; ++i)
		{
            // Ignore forward and backward slashes
            if (data[i] == ForwardSlashSeparator || data[i] == BackSlashSeparator)
            {
                continue;
            }

			const auto next = std::size_t(data[i] );
			acc = ResultT((acc ^ next) * Prime);
		}
		this->state_ = acc;
	}

    constexpr void
        apply_for_path(const char* const data, const std::size_t size) noexcept
    {
        auto acc = this->state_;
        for (auto i = std::size_t{}; i < size; ++i)
        {
            // Ignore forward and backward slashes
            if (data[i] == ForwardSlashSeparator || data[i] == BackSlashSeparator)
            {
                continue;
            }

            const auto next = std::size_t(data[i]);
            acc = ResultT((acc ^ next) * Prime);
        }
        this->state_ = acc;
    }

	constexpr void
        apply(const char *const data) noexcept
	{
		auto acc = this->state_;
		for (auto i = std::size_t{}; data[i] != 0; ++i)
		{
			const auto next = std::size_t( data[i]);
			acc = ResultT((acc ^ next) * Prime);
		}
		this->state_ = acc;
	}

    constexpr void
        apply_for_path(const char* const data) noexcept
    {
        auto acc = this->state_;
        for (auto i = std::size_t{}; data[i] != 0; ++i)
        {
            // Ignore forward and backward slashes
            if (data[i] == ForwardSlashSeparator || data[i] == BackSlashSeparator)
            {
                continue;
            }

            const auto next = std::size_t(data[i]);
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

constexpr uint64_t fnv1a(const char *const _str, const uint64_t _size) noexcept
{
	fnv1a_64 hashfn;;
	hashfn.apply(_str, _size);
	return hashfn.digest();
}

constexpr uint64_t fnv1a_path(const char* const _str, const uint64_t _size) noexcept
{
    fnv1a_64 hashfn;;
    hashfn.apply_for_path(_str, _size);
    return hashfn.digest();
}

constexpr uint64_t fnv1a(const char *const _str) noexcept
{
	fnv1a_64 hashfn;;
	hashfn.apply(_str);
	return hashfn.digest();
}

constexpr uint64_t fnv1a_path(const char* const _str) noexcept
{
    fnv1a_64 hashfn;;
    hashfn.apply_for_path(_str);
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
struct FixedSizePath
{
    static char GetDefaultSeparator()
    {
        return DefaultSeparator;
    }

    //////////////////
    // CONSTRUCTORS //
    //////////////////

    // Default
    FixedSizePath() 
    {
        m_PathString.fill(0);
    }
    
    // c string
    FixedSizePath(const char* _str)
	{
        m_PathString.fill(0);
        std::string temp(_str);
        assert(temp.size() < TotalSize);
        std::copy(temp.begin(), temp.end(), m_PathString.data());
    }

    // char
    FixedSizePath(char _char)
    {
        m_PathString.fill(0);
        std::copy(&_char, &_char + sizeof(char), m_PathString.data());
    }

    // std::string
    FixedSizePath(const std::string& _str)
	{
        m_PathString.fill(0);
        assert(_str.size() < TotalSize);
        std::copy(_str.begin(), _str.end(), m_PathString.data());
    }

    // std::filesystem::path
    FixedSizePath(const std::filesystem::path& _path)
    {
        m_PathString.fill(0);
        auto temp_string = _path.string();
        assert(temp_string.size() < TotalSize);
        std::replace(temp_string.begin(), temp_string.end(), BackSlashSeparator, DefaultSeparator);
        std::copy(temp_string.begin(), temp_string.end(), m_PathString.data());
    }

    ////////////////////////////
    // OPERATIONS/CONVERSIONS //
    ////////////////////////////

    // c string conversion
	const char* c_str() const
	{
		return m_PathString.data();
	}

    // c++ string conversion
    std::string string() const
    {
        return std::string(m_PathString.data());
    }

    // c++ filesystem path conversion
    std::filesystem::path filesystem_path() const
    {
        return std::string(m_PathString.data());
    }

    // filename from path -> std::string
    std::string filename() const
    {
        return std::filesystem::path(m_PathString.data()).filename().string();
    }

    // stem from path -> std::string
    std::string stem() const
    {
        return std::filesystem::path(m_PathString.data()).stem().string();
    }

    // extension from path -> std::string
    std::string extension() const
    {
        return std::filesystem::path(m_PathString.data()).extension().string();
    }

    const std::array<char, TotalSize>& get_raw() const
    {
        return m_PathString;
    }

    std::array<char, TotalSize>& get_raw()
    {
        return m_PathString;
    }

    std::string to_raw_data_string() const
    {
        return std::string(m_PathString.data(), m_PathString.size());
    }

    //

    // compare with a c string (fast comparison without casting)
    bool compare(const char* _str) const
    {
        return std::strcmp(_str, m_PathString.data()) == 0;
    }

    // concatenation with another fixed size path
    template <uint32_t OtherSize>
    FixedSizePath& concat(const FixedSizePath<OtherSize>& _other)
    {
        assert(used_size() + _other.used_size() < TotalSize);
        std::copy(_other.m_PathString.begin(), _other.m_PathString.begin() + _other.used_size(), m_PathString.data() + used_size());
        return *this;
    }

    // concatenation with a char
    FixedSizePath& concat(char _other)
    {
        assert(used_size() + 1 < TotalSize);
        std::copy(&_other, &_other + sizeof(char), m_PathString.data() + used_size());
        return *this;
    }

    // concatenation with a c++ string
    FixedSizePath& concat(const std::string& _other)
    {
        assert(used_size() + _other.size() < TotalSize);
        std::copy(_other.begin(), _other.end(), m_PathString.data() + used_size());
        return *this;
    }

    // erase from range
    void erase(std::size_t _begin, std::size_t _end)
    {
        assert(_begin >= 0 && _end < TotalSize);
        std::memcpy(&m_PathString[_begin], &m_PathString[_end], TotalSize - _end);
    }

    ///////////
    // QUERY //
    ///////////

    static size_t available_size()
    {
        return TotalSize;
    }

    size_t used_size() const
    {
        return string().size();
    }

    bool empty() const
    {
        return m_PathString[0] == 0;
    }

    ///////////////
    // OPERATORS //
    ///////////////

    operator bool() const
    {
        return !empty();
    }

    operator const char* () const
    {
        return m_PathString.data();
    }

    // Assignment operators
    FixedSizePath& operator =(const char* _str)
    {
        std::string temp(_str);
        std::copy(temp.begin(), temp.end(), m_PathString.data());
        return *this;
    }
    FixedSizePath& operator =(const std::string _str)
    {
        std::copy(_str.begin(), _str.end(), m_PathString.data());
        return *this;
    }

    template <typename PathType>
    FixedSizePath operator +(const PathType& _other)
    {
        FixedSizePath result = *this;
        return result.concat(_other);
    }

    // fast comparison with a c string (without cast)
    bool operator ==(const char* _other) const
    {
        return compare(_other);
    }

    bool operator ==(const FixedSizePath& _other) const
    {
        return m_PathString == _other.m_PathString;
    }

    bool operator !=(const FixedSizePath& _other) const
    {
        return m_PathString != _other.m_PathString;
    }

    bool operator<(const FixedSizePath& other) const
    {
        return m_PathString > other.m_PathString;
    }

    bool operator()(const FixedSizePath& a, const FixedSizePath& b)
    {
        return a.m_PathString < b.m_PathString;
    }

private:

	std::array<char, TotalSize> m_PathString;
};

template <uint32_t FirstPathSize, uint32_t SecondPathSize>
FixedSizePath<FirstPathSize> operator +(const FixedSizePath<FirstPathSize>& _first, const FixedSizePath<SecondPathSize>& _second)
{
    FixedSizePath<FirstPathSize> result = _first;
    return result.concat(_second);
}

template <uint32_t FirstPathSize>
FixedSizePath<FirstPathSize> operator +(const FixedSizePath<FirstPathSize>& _first, char _second)
{
    FixedSizePath<FirstPathSize> result = _first;
    return result.concat(_second);
}

namespace ns {
    template <uint32_t TotalSize>
    void to_json(nlohmann::json& j, const FixedSizePath<TotalSize>& s) {
        
        j = nlohmann::json{ {"Path", s.GetRaw()}};
    }

    template <uint32_t TotalSize>
    void from_json(const nlohmann::json& j, FixedSizePath<TotalSize>& s) {
        j.at("Path").get_to(s.GetRaw());
    }
} // namespace ns

typedef FixedSizePath<FilePathSize> Path;
typedef uint64_t FileDataPosition;
typedef uint64_t FileDataSize;

namespace ns {
    static void to_json(nlohmann::json& j, const Path& s) {

        j = nlohmann::json{ {"Path", s.get_raw()} };
    }

    static void from_json(const nlohmann::json& j, Path& s) {
        j.at("Path").get_to(s.get_raw());
    }
} // namespace ns

// The hash primitive type
typedef uint64_t HashPrimitive;

// The hash type
struct Hash
{
	Hash() : m_is_empty(true) {}
	Hash(const std::string _str) : m_is_empty(false)
	{
		m_Hash = fnv1a_path(_str.c_str());
		m_Path = _str;
	}
	Hash(const char* _str) : m_is_empty(false)
	{
		m_Hash = fnv1a_path(_str);
		m_Path = _str;
	}
    Hash(Path _path) : m_is_empty(false)
    {
        m_Hash = fnv1a_path(_path.string().c_str());
        m_Path = _path;
    }

	operator HashPrimitive() const
	{
		return m_Hash;
	}

    operator Path() const
    {
        return m_Path;
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

    operator bool() const
    {
        return !m_is_empty;
    }

	// Return the hash value
	const HashPrimitive get_hash_value() const
	{
		return m_Hash;
	}

	// Return the path reference
	const Path& path() const
	{
		return m_Path;
	}

    bool empty() const
    {
        return m_is_empty;
    }

protected:

	// The hash properties
    Path m_Path;
	HashPrimitive m_Hash;
    bool m_is_empty = true;
};

class PacketFileWatcher
{
public:

    PacketFileWatcher(HashPrimitive _file_hash) :
        m_file_hash(_file_hash) {}

    virtual void OnFileChange() = 0;

    HashPrimitive file_hash() const
    {
        return m_file_hash;
    }

private:

    HashPrimitive m_file_hash;
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

// Merge a resource path with a file path, returning a filesystem path to it
static std::filesystem::path MergeSystemPathWithFilePath(std::filesystem::path _system_path, Path _file_path)
{
    return _system_path.string() + std::string("/") + _file_path.string();
}

// Create a path using an external system path, local dir and optionally a target extension, 
// this will result in a path like <local dir> + <external path filename> + <target extension>
static Path CreateLocalPathFromExternal(std::filesystem::path _external_path, Path _local_dir, std::string _extension = "")
{
    return _local_dir.string() + _external_path.stem().string() + (_extension.length() > 0 ? _extension : _external_path.filename().string());
}

// Convert a filesystem path to an internal path using the system resource path
static Path ConvertSystemPathIntoInternalPath(std::filesystem::path _system_path, std::filesystem::path _file_path)
{
    return std::filesystem::relative(_file_path, _system_path).string();
}

static std::vector<std::string> DecomposePath(std::filesystem::path _path)
{
    std::vector<std::string> folders;
    while (_path != "/" && _path != "")
    {
        folders.push_back(_path.filename().string());
        _path = _path.parent_path();
    }
    return folders;
}

// Compare 2 path filenames
static bool CompareFilenames(Path _first, Path _second)
{
    return _first.string() == _second.string();
}

typedef std::function<bool(
    Path,
    std::vector<uint8_t>&&,
    std::vector<uint8_t>&&,
    std::vector<uint8_t>&&,
    std::vector<uint8_t>&&,
    std::vector<uint8_t>&&,
    std::set<Path>&&,
    FileWriteFlags)> FileWriteCallback;

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

PacketDevelopmentNamespaceEnd(Packet)

namespace std 
{
    template <>
    struct hash<PacketDevelopmentNamespace(Packet)::Hash>
    {
        std::size_t operator()(const PacketDevelopmentNamespace(Packet)::Hash& k) const
        {
            return static_cast<std::size_t>(k.get_hash_value());
        }
    };
}
