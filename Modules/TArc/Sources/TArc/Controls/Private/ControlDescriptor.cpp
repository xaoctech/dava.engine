#include "TArc/Controls/ControlDescriptor.h"

namespace DAVA
{
DescriptorNode::DescriptorNode()
    : fieldName("")
{
}

DescriptorNode& DescriptorNode::operator=(const char* name)
{
    fieldName = FastName(name);
    return *this;
}

DescriptorNode& DescriptorNode::operator=(const String& name)
{
    fieldName = FastName(name);
    return *this;
}

DescriptorNode& DescriptorNode::operator=(const FastName& name)
{
    fieldName = name;
    return *this;
}
} // namespace DAVA