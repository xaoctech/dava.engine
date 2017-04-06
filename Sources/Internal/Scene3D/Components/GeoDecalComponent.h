#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Debug/DVAssert.h"
#include "Entity/Component.h"
#include "Base/Introspection.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class GeoDecalComponent : public Component
{
public:
    struct Config
    {
        float32 projectionOffset = 1.4f / 200.0f;
        float32 perTriangleOffset = 1.4f / 200.0f;
        AABBox3 boundingBox = AABBox3(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f));
        FilePath image;

        bool operator==(const Config&) const;
        bool operator!=(const Config&) const;
        void invalidate();
    };

public:
    IMPLEMENT_COMPONENT_TYPE(GEO_DECAL_COMPONENT);

    GeoDecalComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    const Config& GetConfig() const;

private:
    Config config;

public:
#define IMPL_PROPERTY(T, Name, varName) \
    const T& Get##Name() const { return config.varName; } \
    void Set##Name(const T& value) { config.varName = value; }

    IMPL_PROPERTY(FilePath, DecalImage, image);
    IMPL_PROPERTY(AABBox3, BoundingBox, boundingBox);
    IMPL_PROPERTY(float32, PerTriangleOffset, perTriangleOffset);
    IMPL_PROPERTY(float32, ProjectionOffset, projectionOffset);
#undef IMPL_PROPERTY

    INTROSPECTION_EXTEND(GeoDecalComponent, Component,
                         PROPERTY("Bounding Box", "Bounding Box", GetBoundingBox, SetBoundingBox, I_VIEW | I_EDIT | I_SAVE)
                         PROPERTY("Decal image", "Decal image", GetDecalImage, SetDecalImage, I_VIEW | I_EDIT | I_SAVE)
                         PROPERTY("Projection offset", "Projection offset", GetProjectionOffset, SetProjectionOffset, I_VIEW | I_EDIT | I_SAVE)
                         PROPERTY("Per-triangle offset", "Per-triangle offset", GetPerTriangleOffset, SetPerTriangleOffset, I_VIEW | I_EDIT | I_SAVE)
                         )
    DAVA_VIRTUAL_REFLECTION(GeoDecalComponent, Component);
};

inline const GeoDecalComponent::Config& GeoDecalComponent::GetConfig() const
{
    return config;
}

inline bool GeoDecalComponent::Config::operator==(const GeoDecalComponent::Config& r) const
{
    return (boundingBox == r.boundingBox) && (image == r.image) && (projectionOffset == r.projectionOffset) &&
    (perTriangleOffset == r.perTriangleOffset);
}

inline bool GeoDecalComponent::Config::operator!=(const GeoDecalComponent::Config& r) const
{
    return (boundingBox != r.boundingBox) || (image != r.image) || (projectionOffset != r.projectionOffset) ||
    (perTriangleOffset != r.perTriangleOffset);
}

inline void GeoDecalComponent::Config::invalidate()
{
    boundingBox.Empty();
    image = FilePath();
}
}
