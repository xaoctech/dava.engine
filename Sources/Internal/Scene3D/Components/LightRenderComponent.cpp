#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Components/LightRenderComponent.h"
#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(LightRenderComponent)
{
    ReflectionRegistrator<LightRenderComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

LightRenderComponent::LightRenderComponent()
    : object(new LightRenderObject())
{
}

void LightRenderComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
}

void LightRenderComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
}

Component* LightRenderComponent::Clone(Entity* toEntity)
{
    LightComponent* lightComponent = GetLightComponent(toEntity);

    LightRenderComponent* result = new LightRenderComponent;
    result->SetLight(lightComponent ? lightComponent->GetLightObject() : nullptr);
    result->SetEntity(toEntity);
    return result;
}

void LightRenderComponent::SetLight(Light* l)
{
    if (light != l)
    {
        SafeRelease(light);
        light = SafeRetain(l);
    }

    object->SetLight(light);
    GlobalEventSystem::Instance()->Event(this, EventSystem::LIGHT_PROPERTIES_CHANGED);
}

LightRenderObject* LightRenderComponent::GetRenderObject() const
{
    return object.get();
}
};
