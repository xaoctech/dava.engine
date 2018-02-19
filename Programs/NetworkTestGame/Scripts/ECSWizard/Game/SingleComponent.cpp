#include "TEMPLATESingleComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(TEMPLATESingleComponent)
{
    ReflectionRegistrator<TEMPLATESingleComponent>::Begin()[DAVA::M::Replicable(DAVA::M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .End();
}

void TEMPLATESingleComponent::Clear()
{
    //should bever be cleared during game
}
