#include "REPlatform/Commands/EntityAddCommand.h"

#include <Scene3D/Entity.h>
#include <Scene3D/Components/SwitchComponent.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Utils/StringFormat.h>

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
EntityAddCommand::EntityAddCommand(Entity* _entityToAdd, Entity* toParent)
    : RECommand(Format("Add Entity %s", _entityToAdd->GetName().c_str()))
    , entityToAdd(_entityToAdd)
    , parentToAdd(toParent)
{
    SafeRetain(entityToAdd);
}

EntityAddCommand::~EntityAddCommand()
{
    SafeRelease(entityToAdd);
}

void EntityAddCommand::Undo()
{
    if (nullptr != parentToAdd && nullptr != entityToAdd)
    {
        parentToAdd->RemoveNode(entityToAdd);
    }
}

void EntityAddCommand::Redo()
{
    if (parentToAdd)
    {
        parentToAdd->AddNode(entityToAdd);

        //Workaround for correctly adding of switch
        SwitchComponent* sw = GetSwitchComponent(entityToAdd);
        if (sw)
        {
            sw->SetSwitchIndex(sw->GetSwitchIndex());
        }
    }
}

Entity* EntityAddCommand::GetEntity() const
{
    return entityToAdd;
}

DAVA_VIRTUAL_REFLECTION_IMPL(EntityAddCommand)
{
    ReflectionRegistrator<EntityAddCommand>::Begin()
    .End();
}
} // namespace DAVA
