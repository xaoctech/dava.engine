#include "Commands2/EntityAddCommand.h"
#include "Commands2/RECommandIDs.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"

#include "Utils/StringFormat.h"

EntityAddCommand::EntityAddCommand(DAVA::Entity* entityToAdd, DAVA::Entity* parentToAdd, DAVA::Entity* insertBefore)
    : RECommand(CMDID_ENTITY_ADD, DAVA::Format("Add Entity %s", entityToAdd->GetName().c_str()))
    , entityToAdd(entityToAdd)
    , parentToAdd(parentToAdd)
    , insertBefore(insertBefore)
{
    SafeRetain(entityToAdd);
    DVASSERT(parentToAdd != nullptr);
    DVASSERT(entityToAdd != nullptr);
}

EntityAddCommand::~EntityAddCommand()
{
    SafeRelease(entityToAdd);
}

void EntityAddCommand::Undo()
{
    parentToAdd->RemoveNode(entityToAdd);
}

void EntityAddCommand::Redo()
{
    if (insertBefore != nullptr)
    {
        parentToAdd->InsertBeforeNode(entityToAdd, insertBefore);
    }
    else
    {
        parentToAdd->AddNode(entityToAdd);
    }

    //Workaround for correctly adding of switch
    DAVA::SwitchComponent* sw = GetSwitchComponent(entityToAdd);
    if (sw != nullptr)
    {
        sw->SetSwitchIndex(sw->GetSwitchIndex());
    }
}

DAVA::Entity* EntityAddCommand::GetEntity() const
{
    return entityToAdd;
}
