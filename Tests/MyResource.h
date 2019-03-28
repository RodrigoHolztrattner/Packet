#pragma once

#include "..\Packet\Packet.h"

class MyResource : public Packet::Resource
{
public:

    MyResource();

    ~MyResource();

    bool OnConstruct(Packet::ResourceData& _data, uint32_t _buildFlags, uint32_t _flags) final;

    bool OnExternalConstruct(Packet::ResourceData& _data, uint32_t _buildFlags, uint32_t _flags, void* _customData) final;

    bool OnDelete(Packet::ResourceData& _data) final;
};