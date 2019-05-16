////////////////////////////////////////////////////////////////////////////////
// Filename: Packet.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"

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

typedef __development__Packet::PacketResource				     Resource;
typedef __development__Packet::PacketResourceFactory		     ResourceFactory;
typedef __development__Packet::PacketResourceExternalConstructor ResourceExternalConstructor;

typedef __development__Packet::PacketResourceManager		     ResourceManager;
typedef __development__Packet::PacketSystem					     System;

typedef __development__Packet::OperationMode				     OperationMode;
typedef __development__Packet::ReferenceFixer				     ReferenceFixer;
typedef __development__Packet::Hash							     Hash;
typedef __development__Packet::PacketResourceData			     ResourceData;
typedef __development__Packet::PacketResourceBuildInfo		     ResourceBuildInfo;
typedef __development__Packet::ThreadIndexRetrieveMethod	     ThreadIndexRetrieveMethod;

template <typename ResourceClass>
using ResourceReference          = __development__Packet::PacketResourceReference<ResourceClass>;

// Packet
PacketNamespaceEnd(Packet)