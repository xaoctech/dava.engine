#include "TEMPLATEComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Scene3D/Entity.h>

using namespace DAVA;
REGISTER_CLASS(TEMPLATEComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(TEMPLATEComponent)
{
    ReflectionRegistrator<TEMPLATEComponent>::Begin()
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
