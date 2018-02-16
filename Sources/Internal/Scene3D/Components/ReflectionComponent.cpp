#include "Reflection/Reflection.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Components/ReflectionComponent.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/ReflectionSystem.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(ReflectionComponent)
{
    ReflectionRegistrator<ReflectionComponent>::Begin()
    .ConstructorByPointer()
    .Field("type", &ReflectionComponent::GetReflectionType, &ReflectionComponent::SetReflectionType)[M::DisplayName("type"), M::EnumT<ReflectionProbe::ProbeType>()]
    .Field("capturePosition", &ReflectionComponent::GetCapturePosition, &ReflectionComponent::SetCapturePosition)[M::DisplayName("Capture Position")]
    .Field("captureSize", &ReflectionComponent::GetCaptureSize, &ReflectionComponent::SetCaptureSize)[M::DisplayName("Capture Size")]
    .Field("debugDraw", &ReflectionComponent::GetDebugDrawEnabled, &ReflectionComponent::SetDebugDrawEnabled)[M::DisplayName("Render Debug Info")]
    .Field("reflectionsMap", &ReflectionComponent::GetReflectionsMap, &ReflectionComponent::SetReflectionsMap)[M::DisplayName("Reflections Map")]
    .End();
}

Component* ReflectionComponent::Clone(Entity* toEntity)
{
    ReflectionComponent* component = new ReflectionComponent();
    component->SetEntity(toEntity);
    component->probeType = probeType;
    component->capturePosition = capturePosition;
    component->captureSize = captureSize;
    return component;
}

void ReflectionComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    if (NULL != archive)
    {
        archive->SetUInt32("rc.type", static_cast<uint32>(probeType));
        archive->SetVector3("rc.capturePosition", capturePosition);
        archive->SetVector3("rc.captureSize", captureSize);
        archive->SetByteArray("rc.sh", reinterpret_cast<uint8*>(sphericalHarmonics), sizeof(Vector4) * 9);
        archive->SetString("rc.map", reflectionsMap.GetRelativePathname(serializationContext->GetScenePath()));
    }
}

void ReflectionComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (NULL != archive)
    {
        probeType = static_cast<ReflectionProbe::ProbeType>(archive->GetUInt32("rc.type", static_cast<uint32>(probeType)));
        capturePosition = archive->GetVector3("rc.capturePosition", capturePosition);
        captureSize = archive->GetVector3("rc.captureSize", captureSize);

        String envMap = archive->GetString("rc.map");
        if (!envMap.empty())
        {
            reflectionsMap = serializationContext->GetScenePath() + envMap;
        }

        const uint8* currentSh = reinterpret_cast<const uint8*>(sphericalHarmonics);
        const uint8* sh = archive->GetByteArray("rc.sh", currentSh);
        if ((sh != nullptr) && (sh != currentSh))
        {
            memcpy(sphericalHarmonics, sh, sizeof(Vector4) * 9);
        }
    }
    Component::Deserialize(archive, serializationContext);
}

const Vector4* ReflectionComponent::GetSphericalHarmonics() const
{
    return sphericalHarmonics;
}

void ReflectionComponent::SetSphericalHarmonics(Vector4 sh[9])
{
    memcpy(sphericalHarmonics, sh, sizeof(Vector4) * 9);
}
};
