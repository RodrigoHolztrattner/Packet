#include "MyFactory.h"
#include "MyResource.h"

MyFactory::MyFactory() : Packet::ResourceFactory()
{
}

MyFactory::~MyFactory()
{
}

std::unique_ptr<Packet::Resource> MyFactory::RequestObject(const std::vector<uint8_t>& _resource_data)
{
    return std::unique_ptr<Packet::Resource>(new MyResource());
}

void MyFactory::ReleaseObject(std::unique_ptr<Packet::Resource> _object)
{
    _object.reset();
}
