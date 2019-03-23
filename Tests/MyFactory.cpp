#include "MyFactory.h"
#include "MyResource.h"
#include "MyInstance.h"

MyFactory::MyFactory() : Packet::ResourceFactory()
{
}

MyFactory::~MyFactory()
{
}

std::unique_ptr<Packet::ResourceInstance> MyFactory::RequestInstance(Packet::Hash _hash, 
                                                                     Packet::ResourceManager* _resourceManager)
{
    return std::unique_ptr<Packet::ResourceInstance>(new MyInstance(_hash, _resourceManager, this));
}

void MyFactory::ReleaseInstance(std::unique_ptr<Packet::ResourceInstance> _instance)
{
    _instance.reset();
}

std::unique_ptr<Packet::Resource> MyFactory::RequestObject()
{
    return std::unique_ptr<Packet::Resource>(new MyResource());
}

void MyFactory::ReleaseObject(std::unique_ptr<Packet::Resource> _object)
{
    _object.reset();
}

bool MyFactory::AllocateData(Packet::ResourceData& _data, uint64_t _total)
{
    _data.AllocateMemory(_total);

    return true;
}

void MyFactory::DeallocateData(Packet::ResourceData& _data)
{
    _data.DeallocateMemory();
}