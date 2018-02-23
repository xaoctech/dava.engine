#include "FileSystem/KeyedArchive.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/LightmapComponent.h"
#include "Scene3D/Components/SingleComponents/LightmapSingleComponent.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(LightmapComponent)
{
    ReflectionRegistrator<LightmapComponent>::Begin()
    .ConstructorByPointer()
    .Field("params", &LightmapComponent::params)[M::DisplayName("Lightmap Properties")]
    .End();
}

Component* LightmapComponent::Clone(Entity* toEntity)
{
    LightmapComponent* component = new LightmapComponent();
    component->SetEntity(toEntity);
    component->params = params;

    for (LightmapParam& p : component->params)
        p.component = component;

    return component;
}

void LightmapComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    uint32 paramsCount = uint32(params.size());
    archive->SetUInt32("lightmapParamsCount", paramsCount);
    for (uint32 p = 0; p < paramsCount; ++p)
    {
        const LightmapParam& param = params[p];
        archive->SetUInt32(Format("lightmapSize%u", p), uint32(param.lightmapSize));
        archive->SetBool(Format("castShadow%u", p), param.castShadow);
        archive->SetBool(Format("receiveShadow%u", p), param.receiveShadow);
    }
}

void LightmapComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);

    uint32 paramsCount = archive->GetUInt32("lightmapParamsCount");
    SetParamsCount(paramsCount);
    for (uint32 p = 0; p < paramsCount; ++p)
    {
        LightmapParam& param = params[p];

        param.lightmapSize = float32(archive->GetUInt32(Format("lightmapSize%u", p)));
        param.castShadow = archive->GetBool(Format("castShadow%u", p));
        param.receiveShadow = archive->GetBool(Format("receiveShadow%u", p));
    }
}

uint32 LightmapComponent::GetParamsCount() const
{
    return uint32(params.size());
}

void LightmapComponent::SetParamsCount(uint32 count)
{
    params.resize(count);
    for (LightmapParam& p : params)
        p.component = this;
}

LightmapComponent::LightmapParam& LightmapComponent::GetLightmapParam(uint32 index)
{
    DVASSERT(index < GetParamsCount());
    return params[index];
}

uint32 LightmapComponent::LightmapParam::GetLightmapSize() const
{
    return uint32(lightmapSize);
}

void LightmapComponent::LightmapParam::SetLightmapSize(uint32 size)
{
    if (GetLightmapSize() == size)
        return;

    lightmapSize = float32(size);

    LightmapSingleComponent* lsc = GetSingletonComponent();
    if (lsc != nullptr)
        lsc->lightmapSizeChanged.emplace_back(component);
}

const float32* LightmapComponent::LightmapParam::GetLightmapSizePtr() const
{
    return &lightmapSize;
}

bool LightmapComponent::LightmapParam::IsCastShadow() const
{
    return castShadow;
}

void LightmapComponent::LightmapParam::SetCastShadow(bool cast)
{
    if (castShadow == cast)
        return;

    castShadow = cast;

    LightmapSingleComponent* lsc = GetSingletonComponent();
    if (lsc != nullptr)
        lsc->shadowCasterParamChanged.emplace_back(component);
}

bool LightmapComponent::LightmapParam::IsReceiveShadow() const
{
    return receiveShadow;
}

void LightmapComponent::LightmapParam::SetReceiveShadow(bool receive)
{
    if (receiveShadow == receive)
        return;

    receiveShadow = receive;

    LightmapSingleComponent* lsc = GetSingletonComponent();
    if (lsc != nullptr)
        lsc->shadowReceiverParamChanged.emplace_back(component);
}

LightmapSingleComponent* LightmapComponent::LightmapParam::GetSingletonComponent() const
{
    if (component != nullptr && component->GetEntity() != nullptr && component->GetEntity()->GetScene() != nullptr)
        return component->GetEntity()->GetScene()->GetSingletonComponent<LightmapSingleComponent>();
    return nullptr;
}
}
