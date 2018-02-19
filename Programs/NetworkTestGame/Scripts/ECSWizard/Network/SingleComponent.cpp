#include "TEMPLATESingleComponent.h"
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(TEMPLATESingleComponent)
{
    ReflectionRegistrator<TEMPLATESingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

TEMPLATESingleComponent::TEMPLATESingleComponent()
{
}

TEMPLATESingleComponent::~TEMPLATESingleComponent()
{
}

void TEMPLATESingleComponent::Clear()
{
}
}
