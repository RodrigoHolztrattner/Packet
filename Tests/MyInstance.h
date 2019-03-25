#pragma once

#include "..\Packet\Packet.h"

class MyResource;

class MyInstance : public Packet::ResourceInstance
{
public:

    MyInstance(Packet::Hash& _hash,
               Packet::ResourceManager* _resourceManager,
               Packet::ResourceFactory* _factoryPtr);
    ~MyInstance();

protected:

    void OnConstruct() final;
    void OnDependenciesFulfilled() final;
    void OnReset() final;
};