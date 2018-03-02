////////////////////////////////////////////////////////////////////////////////
// Filename: Packet.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"
#include "PacketObject.h"
#include "PacketObjectIterator.h"
#include "PacketFileManager.h"
#include "PacketFile.h"
#include "PacketFileReference.h"

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

typedef Packet::PacketObject			Object;
typedef Packet::PacketObjectIterator	Iterator;
typedef Packet::PacketFileManager		FileManager;
typedef Packet::PacketFile				File;
typedef Packet::PacketFileReference		FileReference;

// Packet
PacketNamespaceEnd(Packet)