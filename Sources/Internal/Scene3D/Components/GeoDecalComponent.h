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

    const AABBox3& GetBoundingBox() const;
    void SetBoundingBox(const AABBox3& box);

    const FilePath& GetDecalImage() const;
    void SetDecalImage(const FilePath& image);

    const Config& GetConfig() const;

private:
    Config config;

public:
    INTROSPECTION_EXTEND(GeoDecalComponent, Component,
                         PROPERTY("Bounding Box", "Bounding Box", GetBoundingBox, SetBoundingBox, I_VIEW | I_EDIT | I_SAVE)
                         PROPERTY("Decal image", "Decal image", GetDecalImage, SetDecalImage, I_VIEW | I_EDIT | I_SAVE)
                         )
    DAVA_VIRTUAL_REFLECTION(GeoDecalComponent, Component);
};

inline const AABBox3& GeoDecalComponent::GetBoundingBox() const
{
    return config.boundingBox;
}

inline void GeoDecalComponent::SetBoundingBox(const AABBox3& box)
{
    config.boundingBox = box;
}

inline const FilePath& GeoDecalComponent::GetDecalImage() const
{
    return config.image;
}

inline void GeoDecalComponent::SetDecalImage(const FilePath& image)
{
    config.image = image;
}

inline const GeoDecalComponent::Config& GeoDecalComponent::GetConfig() const
{
    return config;
}

inline bool GeoDecalComponent::Config::operator==(const GeoDecalComponent::Config& r) const
{
    return (boundingBox == r.boundingBox) && (image == r.image);
}

inline bool GeoDecalComponent::Config::operator!=(const GeoDecalComponent::Config& r) const
{
    return (boundingBox != r.boundingBox) || (image != r.image);
}

inline void GeoDecalComponent::Config::invalidate()
{
    boundingBox.Empty();
    image = FilePath();
}
}
