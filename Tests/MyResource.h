#pragma once

#include "..\Packet\Packet.h"

class MyResource : public Packet::Resource
{
public:

    MyResource();

    ~MyResource();

    bool OnLoad(Packet::ResourceData& _data, uint32_t _buildFlags, uint32_t _flags) final;

    bool OnDelete(Packet::ResourceData& _data) final;

    void OnConstruct() final;

    void OnExternalConstruct() final;
};