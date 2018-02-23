#include "Base/BaseTypes.h"
#include "FileSystem/KeyedArchive.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Scene3D/Components/MeshComponent.h"
#include "Scene3D/Components/LandscapeComponent.h"
#include "Scene3D/Components/LightmapDataComponent.h"
#include "Scene3D/Components/LightmapComponent.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"
#include "Render/Texture.h"
#include "Render/Highlevel/Mesh.h"
#include "Render/Highlevel/Landscape.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(LightmapDataComponent)
{
    ReflectionRegistrator<LightmapDataComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

LightmapDataComponent::~LightmapDataComponent()
{
    for (auto& it : lightmapData)
        SafeRelease(it.second.lightmapTexture);
}

Component* LightmapDataComponent::Clone(Entity* toEntity)
{
    LightmapDataComponent* component = new LightmapDataComponent();
    component->SetEntity(toEntity);
    component->lightmapData = lightmapData;

    for (auto& it : component->lightmapData)
        SafeRetain(it.second.lightmapTexture);

    return component;
}

void LightmapDataComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    uint32 dataIndex = 0;
    for (const auto& data : lightmapData)
    {
        archive->SetString(Format("dataID%u", dataIndex), String(data.first.c_str()));
        archive->SetString(Format("lightmapPath%u", dataIndex), data.second.lightmapPath.GetRelativePathname(serializationContext->GetScenePath()));
        archive->SetVector4(Format("uv%u", dataIndex), data.second.uvOffsetScale);
        ++dataIndex;
    }
    archive->SetUInt32("lightmapDataCount", dataIndex);
}

void LightmapDataComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);

    uint32 dataCount = archive->GetUInt32("lightmapDataCount");
    for (uint32 d = 0; d < dataCount; ++d)
    {
        String dataID = archive->GetString(Format("dataID%u", d));
        if (!dataID.empty())
        {
            LightmapData& data = lightmapData[FastName(dataID.c_str())];
            DVASSERT(data.lightmapTexture == nullptr);

            data.lightmapPath = serializationContext->GetScenePath() + archive->GetString(Format("lightmapPath%u", d));
            data.uvOffsetScale = archive->GetVector4(Format("uv%u", d));
            data.lightmapTexture = Texture::PureCreate(data.lightmapPath);
        }
    }
}

void LightmapDataComponent::SetLightmapData(const FastName& id, const Vector4& uv, const FilePath& lightmapPath)
{
    LightmapData& data = lightmapData[id];
    SafeRelease(data.lightmapTexture);

    data.lightmapPath = lightmapPath;
    data.lightmapTexture = Texture::PureCreate(lightmapPath);
    data.uvOffsetScale = uv;
}

void LightmapDataComponent::RemoveLightmapData()
{
    for (auto& it : lightmapData)
        SafeRelease(it.second.lightmapTexture);
    lightmapData.clear();
}

void LightmapDataComponent::ReloadLightmaps()
{
    for (auto& it : lightmapData)
    {
        Texture* texture = it.second.lightmapTexture;
        if (texture != nullptr)
            texture->Reload();
    }
}

void LightmapDataComponent::RebuildIDs()
{
    lightmapIDs.clear();
    RebuildIDs(String(), GetEntity());
}

void LightmapDataComponent::RebuildIDs(String baseID, Entity* entity)
{
    for (uint32 m = 0; m < entity->GetComponentCount<MeshComponent>(); ++m)
    {
        RebuildIDs(baseID + Format("m%u_", m), entity->GetComponent<MeshComponent>(m));
    }

    for (uint32 l = 0; l < entity->GetComponentCount<LandscapeComponent>(); ++l)
    {
        RebuildIDs(baseID + Format("l%u_", l), entity->GetComponent<LandscapeComponent>(l));
    }

    for (int32 c = 0; c < entity->GetChildrenCount(); ++c)
    {
        RebuildIDs(baseID + Format("e%d_", c), entity->GetChild(c));
    }
}

void LightmapDataComponent::RebuildIDs(String baseID, MeshComponent* meshComponent)
{
    LightmapComponent* lightmapComponent = meshComponent->GetEntity()->GetComponent<LightmapComponent>();
    if (lightmapComponent == nullptr)
        return;

    Mesh* mesh = meshComponent->GetMesh();
    for (uint32 b = 0; b < mesh->GetRenderBatchCount(); ++b)
    {
        if (lightmapComponent->GetLightmapParam(b).IsReceiveShadow())
        {
            FastName id = FastName(baseID + Format("b%u", b));
            lightmapIDs[mesh->GetRenderBatch(b)] = id;
        }
    }
}

void LightmapDataComponent::RebuildIDs(String baseID, LandscapeComponent* landscapeComponent)
{
    LightmapComponent* lightmapComponent = landscapeComponent->GetEntity()->GetComponent<LightmapComponent>();
    if (lightmapComponent == nullptr)
        return;

    if (lightmapComponent->GetLightmapParam(0).IsReceiveShadow())
    {
        FastName id = FastName(baseID + "l");
        lightmapIDs[landscapeComponent->GetLandscape()] = id;
    }
}

const FastName& LightmapDataComponent::GetLightmapID(BaseObject* object)
{
    auto found = lightmapIDs.find(object);
    if (found != lightmapIDs.end())
        return found->second;

    static FastName emptyID;
    return emptyID;
}
}
