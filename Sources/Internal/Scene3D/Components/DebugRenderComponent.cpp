#include "Scene3D/Components/DebugRenderComponent.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(DebugRenderComponent)
{
    ReflectionRegistrator<DebugRenderComponent>::Begin()[M::CantBeCreatedManualyComponent()]
    .ConstructorByPointer()
    .Field("renderObject", &DebugRenderComponent::renderObject)[M::DisplayName("Render Object")]
    .End();
}

DebugRenderComponent::DebugRenderComponent(RenderObject* _object)
{
    renderObject = SafeRetain(_object);
}

DebugRenderComponent::~DebugRenderComponent()
{
    SafeRelease(renderObject);
}

void DebugRenderComponent::SetRenderObject(RenderObject* _renderObject)
{
    SafeRelease(renderObject);
    renderObject = SafeRetain(_renderObject);
}

RenderObject* DebugRenderComponent::GetRenderObject() const
{
    return renderObject;
}

Component* DebugRenderComponent::Clone(Entity* toEntity)
{
    DebugRenderComponent* component = new DebugRenderComponent();
    component->SetEntity(toEntity);

    if (nullptr != renderObject)
    {
        component->renderObject = renderObject->Clone(component->renderObject);
    }
    return component;
}

void DebugRenderComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    // Don't need to save
}

void DebugRenderComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    // Don't need to load
}
};
