#include "MyResource.h"

MyResource::MyResource()
{
}

MyResource::~MyResource()
{
}


bool MyResource::OnConstruct(Packet::ResourceData& _data, uint32_t _buildFlags, uint32_t _flags)
{
    return true;
}

bool MyResource::OnExternalConstruct(Packet::ResourceData& _data, uint32_t _buildFlags, uint32_t _flags, void* _customData)
{
    return true;
}

bool MyResource::OnDelete(Packet::ResourceData& _data)
{
    return true;
}
