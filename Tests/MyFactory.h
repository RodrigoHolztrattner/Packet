#pragma once

#include "..\Packet\Packet.h"

class MyResource;
class MyInstance;

class MyFactory : public Packet::ResourceFactory
{
public:

    MyFactory();
    ~MyFactory();

    std::unique_ptr<Packet::ResourceInstance> RequestInstance(Packet::Hash _hash,
                                                              Packet::ResourceManager* _resourceManager);
    void ReleaseInstance(std::unique_ptr<Packet::ResourceInstance> _instance) final;
    std::unique_ptr<Packet::Resource> RequestObject() final;
    void ReleaseObject(std::unique_ptr<Packet::Resource> _object) final;
    bool AllocateData(Packet::ResourceData& _data, uint64_t _total) final;
    void DeallocateData(Packet::ResourceData& _data) final;
};