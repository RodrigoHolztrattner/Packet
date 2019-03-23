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
}

void MyInstance::OnDependenciesFulfilled()
{
}

void MyInstance::OnReset()
{
}