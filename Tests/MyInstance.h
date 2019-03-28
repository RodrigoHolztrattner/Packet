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

    // Return the number of times xxx method was called
    uint32_t GetOnConstructTotalCalls()             const;
    uint32_t GetOnDependenciesFulfilledTotalCalls() const;
    uint32_t GetOnDeleteTotalCalls()                const;

protected:

    void OnConstruct() final;
    void OnDependenciesFulfilled() final;
    void OnDelete() final;

private:

    uint32_t m_OnConstructTotalCalls             = 0;
    uint32_t m_OnDependenciesFulfilledTotalCalls = 0;
    uint32_t m_OnDeleteTotalCalls                = 0;
};