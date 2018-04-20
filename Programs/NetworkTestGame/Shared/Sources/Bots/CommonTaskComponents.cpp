#include "CommonTaskComponents.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(WaitTaskComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(WaitTaskComponent)
{
    ReflectionRegistrator<WaitTaskComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

Component* WaitTaskComponent::Clone(Entity* toEntity)
{
    WaitTaskComponent* component = new WaitTaskComponent();
    component->SetEntity(toEntity);
    return component;
}

WaitTaskComponent::WaitTaskComponent(Type type_, float time_)
    : time(time_)
    , type(type_)
{
}

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
    : first(first_)
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
