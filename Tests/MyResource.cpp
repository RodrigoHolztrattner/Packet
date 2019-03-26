#include "MyResource.h"

MyResource::MyResource()
{
}

MyResource::~MyResource()
{
}

bool MyResource::OnLoad(Packet::ResourceData&, uint32_t, uint32_t)
{
    return true;
}

bool MyResource::OnDelete(Packet::ResourceData& _data)
{
    return true;
}

void MyResource::OnConstruct()
{
};

void MyResource::OnExternalConstruct(void* _data)
{
};