#include "Scene3D/Components/EnvironmentComponent.h"
#include "Scene3D/Entity.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(EnvironmentComponent)
{
    ReflectionRegistrator<EnvironmentComponent>::Begin()
    .ConstructorByPointer()
    .Field("fogDistanceScale", &EnvironmentComponent::GetFogDistanceScale, &EnvironmentComponent::SetFogDistanceScale)[M::DisplayName("Scattering Distance Scale"), M::Range(0.0f, 100.0f, 0.01f)]
    .Field("fogTurbidity", &EnvironmentComponent::GetFogTurbidity, &EnvironmentComponent::SetFogTurbidity)[M::DisplayName("Atmosphere Tubidity"), M::Range(0.0f, 100.0f, 0.01f)]
    .Field("fogAnisotropy", &EnvironmentComponent::GetFogAnisotropy, &EnvironmentComponent::SetFogAnisotropy)[M::DisplayName("Scattering Anisotropy"), M::Range(-1.0f, 1.0f, 0.01f)]
    .End();
}

Component* EnvironmentComponent::Clone(Entity* toEntity)
{
    EnvironmentComponent* result = new EnvironmentComponent();
    result->SetEntity(toEntity);
    result->fogDistanceScale = fogDistanceScale;
    result->fogTurbidity = fogTurbidity;
    result->fogAnisotropy = fogAnisotropy;
    return result;
}

void EnvironmentComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (archive)
    {
        archive->SetFloat("fog.scale", fogDistanceScale);
        archive->SetFloat("fog.turbidity", fogTurbidity);
        archive->SetFloat("fog.anisotropy", fogAnisotropy);
    }
    Component::Serialize(archive, serializationContext);
}

void EnvironmentComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
    if (archive)
    {
        fogDistanceScale = archive->GetFloat("fog.scale", fogDistanceScale);
        fogTurbidity = archive->GetFloat("fog.turbidity", fogTurbidity);
        fogAnisotropy = archive->GetFloat("fog.anisotropy", fogAnisotropy);
    }
}
}
