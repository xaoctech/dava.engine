#include "Reflection/Reflection.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Components/ReflectionComponent.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/ReflectionSystem.h"

/*ENUM_DECLARE(DAVA::ReflectionProbe::eType)
{
    ENUM_ADD_DESCR(DAVA::ReflectionProbe::eType::TYPE_NONE, "None");
    ENUM_ADD_DESCR(DAVA::ReflectionProbe::eType::TYPE_GLOBAL, "Global");
    ENUM_ADD_DESCR(DAVA::ReflectionProbe::eType::TYPE_STATIC_CUBEMAP, "Static Cubemap");
}*/

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(ReflectionComponent)
{
    ReflectionRegistrator<ReflectionComponent>::Begin()
    .ConstructorByPointer()
    .Field("type", &ReflectionComponent::GetReflectionType, &ReflectionComponent::SetReflectionType)[M::DisplayName("type"), M::EnumT<ReflectionProbe::eType>()]
    .Field("capturePosition", &ReflectionComponent::GetCapturePosition, &ReflectionComponent::SetCapturePosition)[M::DisplayName("Capture Position")]
    .Field("captureSize", &ReflectionComponent::GetCaptureSize, &ReflectionComponent::SetCaptureSize)[M::DisplayName("Capture Size")]

    .Field("debugDraw", &ReflectionComponent::GetDebugDrawEnabled, &ReflectionComponent::SetDebugDrawEnabled)[M::DisplayName("Render Debug Info")]
    .End();
}

Component* ReflectionComponent::Clone(Entity* toEntity)
{
    ReflectionComponent* component = new ReflectionComponent();
    component->SetEntity(toEntity);
    component->type = type;
    component->capturePosition = capturePosition;
    component->captureSize = captureSize;

    return component;
}

void ReflectionComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    if (NULL != archive)
    {
        archive->SetUInt32("rc.type", type);
        archive->SetVector3("rc.capturePosition", capturePosition);
        archive->SetVector3("rc.captureSize", captureSize);
    }
}

void ReflectionComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (NULL != archive)
    {
        type = static_cast<ReflectionProbe::eType>(archive->GetUInt32("rc.type", ReflectionProbe::TYPE_NONE));
        capturePosition = archive->GetVector3("rc.capturePosition", capturePosition);
        captureSize = archive->GetVector3("rc.captureSize", captureSize);
    }
    Component::Deserialize(archive, serializationContext);
}
};
