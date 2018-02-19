#include "TEMPLATEComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

using namespace DAVA;
REGISTER_CLASS(TEMPLATEComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(TEMPLATEComponent)
{
    ReflectionRegistrator<TEMPLATEComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .End();
}

TEMPLATEComponent::TEMPLATEComponent()
{
}

Component* TEMPLATEComponent::Clone(Entity* toEntity)
{
    TEMPLATEComponent* component = new TEMPLATEComponent();
    component->SetEntity(toEntity);
    return component;
}

TEMPLATEComponent::~TEMPLATEComponent()
{
}
