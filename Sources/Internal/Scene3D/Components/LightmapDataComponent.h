#pragma once

#include "Base/BaseMath.h"
#include "Base/FastName.h"
#include "Base/UnordererMap.h"
#include "Entity/Component.h"
#include "FileSystem/FilePath.h"
#include "Reflection/Reflection.h"
#include "Render/Texture.h"
#include "Render/RHI/rhi_Public.h"

namespace DAVA
{
class Entity;
class KeyedArchive;
class RenderBatch;
class RenderObject;
class MeshComponent;
class LandscapeComponent;
class SerializationContext;
class StaticLightingSystem;
class LightmapDataComponent : public Component
{
public:
    LightmapDataComponent() = default;
    ~LightmapDataComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void SetLightmapData(const FastName& id, const Vector4& uv, const FilePath& lightmapPath);
    void RemoveLightmapData();

    void ReloadLightmaps();

    void RebuildIDs();

    const FastName& GetLightmapID(BaseObject* object);

protected:
    void RebuildIDs(String baseID, Entity* entity);
    void RebuildIDs(String baseID, MeshComponent* meshComponent);
    void RebuildIDs(String baseID, LandscapeComponent* meshComponent);

    struct LightmapData
    {
        Vector4 uvOffsetScale;
        FilePath lightmapPath;

        Texture* lightmapTexture = nullptr;
    };

    UnorderedMap<FastName, LightmapData> lightmapData;
    UnorderedMap<BaseObject*, FastName> lightmapIDs;

    DAVA_VIRTUAL_REFLECTION(LightmapDataComponent, Component);

    friend class StaticLightingSystem;
};
}
