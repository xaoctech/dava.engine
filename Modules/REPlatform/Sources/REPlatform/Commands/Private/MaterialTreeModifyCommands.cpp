#include "REPlatform/Commands/MaterialTreeModifyCommands.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
MaterialAddCommand::MaterialAddCommand(NMaterial* _material)
    : RECommand("Add material")
    , material(_material)
{
    SafeRetain(material);
}

MaterialAddCommand::~MaterialAddCommand()
{
    SafeRelease(material);
}

NMaterial* MaterialAddCommand::GetMaterial() const
{
    return material;
}

DAVA_VIRTUAL_REFLECTION_IMPL(MaterialAddCommand)
{
    ReflectionRegistrator<MaterialAddCommand>::Begin()
    .End();
}

//////////////////////////////////////////////////////////////////////////

MaterialRemoveCommand::MaterialRemoveCommand(NMaterial* _material)
    : RECommand("Remove material")
    , material(_material)
    , parent(_material->GetParent())
{
    SafeRetain(material);
    SafeRetain(parent);
}

MaterialRemoveCommand::~MaterialRemoveCommand()
{
    SafeRelease(material);
    SafeRelease(parent);
}

void MaterialRemoveCommand::Undo()
{
    material->SetParent(parent);
}

void MaterialRemoveCommand::Redo()
{
    material->SetParent(nullptr);
}

NMaterial* MaterialRemoveCommand::GetMaterial() const
{
    return material;
}

DAVA_VIRTUAL_REFLECTION_IMPL(MaterialRemoveCommand)
{
    ReflectionRegistrator<MaterialRemoveCommand>::Begin()
    .End();
}

//////////////////////////////////////////////////////////////////////////

MaterialSwitchParentCommand::MaterialSwitchParentCommand(NMaterial* instance, NMaterial* _newParent)
    : RECommand("Switch Material Parent")
{
    DVASSERT(instance);
    DVASSERT(_newParent);

    currentInstance = SafeRetain(instance);
    newParent = SafeRetain(_newParent);
    oldParent = SafeRetain(instance->GetParent());
}

MaterialSwitchParentCommand::~MaterialSwitchParentCommand()
{
    SafeRelease(oldParent);
    SafeRelease(newParent);
    SafeRelease(currentInstance);
}

void MaterialSwitchParentCommand::Redo()
{
    currentInstance->SetParent(newParent);
}

void MaterialSwitchParentCommand::Undo()
{
    currentInstance->SetParent(oldParent);
}

NMaterial* MaterialSwitchParentCommand::GetMaterial() const
{
    return currentInstance;
}

DAVA_VIRTUAL_REFLECTION_IMPL(MaterialSwitchParentCommand)
{
    ReflectionRegistrator<MaterialSwitchParentCommand>::Begin()
    .End();
}
} // namespace DAVA
