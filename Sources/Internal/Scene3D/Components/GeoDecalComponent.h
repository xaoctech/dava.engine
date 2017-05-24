#pragma once

#include "Base/BaseTypes.h"
#include "Base/Introspection.h"
#include "Debug/DVAssert.h"
#include "Entity/Component.h"
#include "Reflection/Reflection.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Render/Highlevel/GeoDecalManager.h"
#include "Render/Material/NMaterial.h"

namespace DAVA
{
class GeoDecalComponent : public Component
{
public:
    enum : uint32
    {
        SuppressMaterialCreation = 1 << 0
    };

public:
    IMPLEMENT_COMPONENT_TYPE(GEO_DECAL_COMPONENT);

    GeoDecalComponent();
    GeoDecalComponent(uint32 flags);

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    const GeoDecalManager::DecalConfig& GetConfig() const;

    bool GetRebakeOnTransform() const;
    void SetRebakeOnTransform(const bool& value);

    void GetDataNodes(Set<DataNode*>& dataNodes) override;

private:
    void ConfigChanged();
    void init(uint32 flags);

private:
    ScopedPtr<NMaterial> dataNodeMaterial;
    GeoDecalManager::DecalConfig config;
    bool rebakeOnTransform = true;

public:
#define IMPL_PROPERTY(T, Name, varName) \
    const T& Get##Name() const { return config.varName; } \
    void Set##Name(const T& value) { config.varName = value; ConfigChanged(); }
    IMPL_PROPERTY(FilePath, DecalAlbedo, albedo);
    IMPL_PROPERTY(FilePath, DecalNormal, normal);
    IMPL_PROPERTY(AABBox3, BoundingBox, boundingBox);
    IMPL_PROPERTY(Vector2, UVScale, uvScale);
    IMPL_PROPERTY(Vector2, UVOffset, uvOffset);
    IMPL_PROPERTY(GeoDecalManager::Mapping, Mapping, mapping);
#undef IMPL_PROPERTY

    INTROSPECTION_EXTEND(GeoDecalComponent, Component, NULL)
    DAVA_VIRTUAL_REFLECTION(GeoDecalComponent, Component);
};

inline const GeoDecalManager::DecalConfig& GeoDecalComponent::GetConfig() const
{
    return config;
}

inline bool GeoDecalComponent::GetRebakeOnTransform() const
{
    return rebakeOnTransform;
}

inline void GeoDecalComponent::SetRebakeOnTransform(const bool& value)
{
    rebakeOnTransform = value;
}
}
