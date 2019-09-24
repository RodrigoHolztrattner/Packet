#pragma once

#include "..\Packet\Packet.h"

class MyResource;
class MyInstance;

class MyFactory : public Packet::ResourceFactory
{
public:

    MyFactory();
    ~MyFactory();

    std::unique_ptr<Packet::Resource> RequestObject(const std::vector<uint8_t>& _resource_data) final;
    void ReleaseObject(std::unique_ptr<Packet::Resource> _object) final;
};