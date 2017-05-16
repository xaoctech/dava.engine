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
    IMPLEMENT_COMPONENT_TYPE(GEO_DECAL_COMPONENT);

    GeoDecalComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    const GeoDecalManager::DecalConfig& GetConfig() const;

    bool GetRebakeOnTransform() const;
    void SetRebakeOnTransform(const bool& value);

    void GetDataNodes(Set<DataNode*>& dataNodes) override;

private:
    ScopedPtr<NMaterial> dataNodeMaterial = ScopedPtr<NMaterial>(new NMaterial());
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
#undef IMPL_PROPERTY

    uint32 GetMapping() const;
    void SetMapping(uint32 value);
    void ConfigChanged();

    INTROSPECTION_EXTEND(GeoDecalComponent, Component,
                         PROPERTY("Bounding Box", "Bounding Box", GetBoundingBox, SetBoundingBox, I_VIEW | I_EDIT | I_SAVE)
                         PROPERTY("Decal albedo", "Decal albedo", GetDecalAlbedo, SetDecalAlbedo, I_VIEW | I_EDIT | I_SAVE)
                         PROPERTY("Decal normal", "Decal normal", GetDecalNormal, SetDecalNormal, I_VIEW | I_EDIT | I_SAVE)
                         PROPERTY("UV Scale", "UV Scale", GetUVScale, SetUVScale, I_VIEW | I_EDIT | I_SAVE)
                         PROPERTY("UV Offset", "UV Offset", GetUVOffset, SetUVOffset, I_VIEW | I_EDIT | I_SAVE)
                         PROPERTY("Texture mapping", InspDesc("Texture mapping", GlobalEnumMap<GeoDecalManager::Mapping>::Instance()), GetMapping, SetMapping, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("Rebake on transform", "Rebake on transform", GetRebakeOnTransform, SetRebakeOnTransform, I_SAVE | I_VIEW | I_EDIT)
                         )
    DAVA_VIRTUAL_REFLECTION(GeoDecalComponent, Component);
};

inline uint32 GeoDecalComponent::GetMapping() const
{
    return config.mapping;
}

inline void GeoDecalComponent::SetMapping(uint32 value)
{
    DVASSERT(value < GeoDecalManager::Mapping::COUNT);
    config.mapping = static_cast<GeoDecalManager::Mapping>(value);
}

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
