#include "CompositeTaskComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(CompositeTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(CompositeTaskComponent)
{
    ReflectionRegistrator<CompositeTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* CompositeTaskComponent::Clone(Entity* toEntity)
{
    CompositeTaskComponent* component = new CompositeTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

CompositeTaskComponent::CompositeTaskComponent(Type type_, BotTaskComponent* first_, BotTaskComponent* second_)
    :
    first(first_)
    , second(second_)
    , type(type_)
{
}

void CompositeTaskComponent::TraverseTaskTree(const TraversalCallback& callback)
{
    callback(first);
    callback(second);
    callback(this);
}
