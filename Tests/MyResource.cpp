#include "MyResource.h"

MyResource::MyResource()
{
}

MyResource::~MyResource()
{
}

bool MyResource::OnLoad(Packet::ResourceData& _data, uint32_t _buildFlags, uint32_t _flags)
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

void MyResource::OnExternalConstruct()
{
};