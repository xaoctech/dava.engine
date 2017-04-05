#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Debug/DVAssert.h"
#include "Entity/Component.h"
#include "Base/Introspection.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class GeoDecalRenderComponent : public Component
{
public:
    IMPLEMENT_COMPONENT_TYPE(GEO_DECAL_RENDER_COMPONENT);

    GeoDecalRenderComponent() = default;
    GeoDecalRenderComponent(RenderObject* ro);
    ~GeoDecalRenderComponent();

    RenderObject* GetRenderObject() const;

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

private:
    RenderObject* renderObject = nullptr;

public:
    INTROSPECTION_EXTEND(GeoDecalRenderComponent, Component,
                         MEMBER(renderObject, "Render Object", I_VIEW)
                         )
    DAVA_VIRTUAL_REFLECTION(GeoDecalRenderComponent, Component);
};
}
