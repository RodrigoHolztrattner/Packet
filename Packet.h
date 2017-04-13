////////////////////////////////////////////////////////////////////////////////
// Filename: PacketLoader.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "..\NamespaceDefinitions.h"

#include "PacketString.h"
#include "PacketFile.h"
#include "PacketDirectory.h"
#include "PacketMode.h"
#include "PacketLoader.h"
#include "PacketManager.h"
#include "PacketIndexLoader.h"

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
NamespaceBegin(Packet)

/////////////
// DEFINES //
/////////////

typedef PacketString				String;
typedef PacketFile					File;
typedef PacketDirectory				Directory;
typedef PacketMode					Mode;
typedef PacketLoader				Loader;
typedef PacketManager				Manager;
typedef PacketIndexLoader			IndexLoader;

// Packet data explorer
NamespaceEnd(Packet)