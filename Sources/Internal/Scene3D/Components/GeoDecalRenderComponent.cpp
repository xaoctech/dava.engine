#include "Scene3D/Components/GeoDecalRenderComponent.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(GeoDecalRenderComponent)
{
    ReflectionRegistrator<GeoDecalRenderComponent>::Begin()[M::CantBeCreatedManualyComponent()]
    .End();
}

GeoDecalRenderComponent::GeoDecalRenderComponent(RenderObject* ro)
    : renderObject(SafeRetain(ro))
{
}

GeoDecalRenderComponent::~GeoDecalRenderComponent()
{
    SafeRelease(renderObject);
}

RenderObject* GeoDecalRenderComponent::GetRenderObject() const
{
    return renderObject;
}

Component* GeoDecalRenderComponent::Clone(Entity* toEntity)
{
    GeoDecalRenderComponent* result = new GeoDecalRenderComponent();
    result->SetEntity(toEntity);
    return result;
}

void GeoDecalRenderComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
}

void GeoDecalRenderComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
}
}
