#include "MyInstance.h"
#include "MyResource.h"

MyInstance::MyInstance(Packet::Hash& _hash,
                       Packet::ResourceManager* _resourceManager,
                       Packet::ResourceFactory* _factoryPtr) :
    Packet::ResourceInstance(_hash,
                             _resourceManager,
                             _factoryPtr)
{
}

MyInstance::~MyInstance()
{
}

void MyInstance::OnConstruct()
{
    m_OnConstructTotalCalls++;
}

void MyInstance::OnDependenciesFulfilled()
{
    m_OnDependenciesFulfilledTotalCalls++;
}

void MyInstance::OnDelete()
{
    m_OnDeleteTotalCalls++;
}

uint32_t MyInstance::GetOnConstructTotalCalls() const
{
    return m_OnConstructTotalCalls;
}

uint32_t MyInstance::GetOnDependenciesFulfilledTotalCalls() const
{
    return m_OnDependenciesFulfilledTotalCalls;
}

uint32_t MyInstance::GetOnDeleteTotalCalls() const
{
    return m_OnDeleteTotalCalls;
}