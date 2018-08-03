////////////////////////////////////////////////////////////////////////////////
// Filename: Packet.h
////////////////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include "PacketConfig.h"

#include "Resource/PacketResource.h"
#include "Resource/PacketResourceInstance.h"
#include "Resource/PacketResourceFactory.h"
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

typedef __development__Packet::PacketResource				Resource;
typedef __development__Packet::PacketResourceInstance		ResourceInstance;
typedef __development__Packet::PacketResourceFactory		ResourceFactory;

typedef __development__Packet::PacketResourceManager		ResourceManager;
typedef __development__Packet::PacketSystem					System;

typedef __development__Packet::OperationMode				OperationMode;
typedef __development__Packet::ReferenceFixer				ReferenceFixer;
typedef __development__Packet::Hash							Hash;
typedef __development__Packet::PacketResourceData			ResourceData;
typedef __development__Packet::PacketResourceBuildInfo		ResourceBuildInfo;
typedef __development__Packet::ThreadIndexRetrieveMethod	ThreadIndexRetrieveMethod;

template <typename InstanceClass>
using ResourceInstancePtr = __development__Packet::PacketResourceInstancePtr<InstanceClass>;

template <typename ResourceClass>
using ResourceReferencePtr = __development__Packet::PacketResourceReferencePtr<ResourceClass>;

// Packet
PacketNamespaceEnd(Packet)