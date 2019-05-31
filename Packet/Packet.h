////////////////////////////////////////////////////////////////////////////////
// Filename: Packet.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"

#include "File/PacketFile.h"
#include "File/PacketFileManager.h"
#include "File/Importer/PacketFileImporter.h"
#include "File/Indexer/PacketFileIndexer.h"
#include "File/Loader/PacketFileLoader.h"
#include "File/Converter/PacketFileConverter.h"
#include "Resource/PacketResource.h"
#include "Resource/Factory/PacketResourceFactory.h"
#include "Resource/PacketResourceManager.h"
#include "PacketSystem.h"

/////////////
// DEFINES //
/////////////

///////////////
// NAMESPACE //
///////////////

// Packet
PacketNamespaceBegin(Packet)

////////////
// GLOBAL //
////////////

static const uint32_t Version                                    = __development__Packet::PacketVersion;
static const uint32_t IconLengthSize                             = __development__Packet::IconLengthSize;
static const uint32_t IconTotalSize                              = __development__Packet::IconTotalSize;
static const uint32_t MaximumPackageSize                         = __development__Packet::MaximumPackageSize;
static const uint32_t MaximumPackageFiles                        = __development__Packet::MaximumPackageFiles;
static const uint32_t CondensedMinorVersion                      = __development__Packet::CondensedMinorVersion;
static const uint32_t CondensedMajorVersion                      = __development__Packet::CondensedMajorVersion;
static const std::string DefaultConverterExtension               = __development__Packet::DefaultConverterExtension;

typedef __development__Packet::OperationMode                     OperationMode;
typedef __development__Packet::BackupFlags                       BackupFlags;
typedef __development__Packet::FilePart                          FilePart;
typedef __development__Packet::FileImportFlagBits                FileImportFlagBits;
typedef __development__Packet::FileWriteFlagBits                 FileWriteFlagBits;
typedef __development__Packet::FileImportFlags                   FileImportFlags;
typedef __development__Packet::FileWriteFlags                    FileWriteFlags;

typedef __development__Packet::PacketFile                        File;
typedef __development__Packet::PacketFileManager                 FileManager;
typedef __development__Packet::PacketFileImporter                FileImporter;
typedef __development__Packet::PacketFileIndexer                 FileIndexer;
typedef __development__Packet::PacketFileLoader                  FileLoader;
typedef __development__Packet::PacketFileWatcher                 FileWatcher;
typedef __development__Packet::PacketFileConverter               FileConverter;

typedef __development__Packet::PacketResource                    Resource;
typedef __development__Packet::PacketResourceFactory             ResourceFactory;
typedef __development__Packet::PacketResourceExternalConstructor ResourceExternalConstructor;

typedef __development__Packet::PacketResourceManager             ResourceManager;
typedef __development__Packet::PacketSystem                      System;

typedef __development__Packet::Hash                              Hash;
typedef __development__Packet::Path                              Path;
typedef __development__Packet::PacketResourceData                ResourceData;
typedef __development__Packet::PacketResourceBuildInfo           ResourceBuildInfo;
typedef __development__Packet::ThreadIndexRetrieveMethod         ThreadIndexRetrieveMethod;

template <typename ResourceClass>
using ResourceReference                                          = __development__Packet::PacketResourceReference<ResourceClass>;

// Packet
PacketNamespaceEnd(Packet)