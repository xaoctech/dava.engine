#include "Scene3D/Components/RuntimeEntityMarkComponent.h"

#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
Component* RuntimeEntityMarkComponent::Clone(Entity* toEntity)
{
    Component* component = new RuntimeEntityMarkComponent();
    component->SetEntity(toEntity);
    return component;
}

DAVA_VIRTUAL_REFLECTION_IMPL(RuntimeEntityMarkComponent)
{
    ReflectionRegistrator<RuntimeEntityMarkComponent>::Begin()[M::NonSerializableComponent(),
                                                               M::NonExportableComponent()]
    .End();
}
} // namespace DAVA
