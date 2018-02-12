#include "REPlatform/Commands/MaterialAssignCommand.h"

#include <Reflection/ReflectedTypeDB.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
MaterialAssignCommand::MaterialAssignCommand(Entity* _entity, NMaterial* _material, const Reflection::Field& _field)
    : RECommand("Assign material")
    , entity(_entity)
    , newMaterial(_material)
    , materialField(_field)
{
    DVASSERT(materialField.ref.GetValueType() == Type::Instance<NMaterial*>());

    SafeRetain(entity);
    SafeRetain(newMaterial);
    oldMaterial = SafeRetain(materialField.ref.GetValue().Get<NMaterial*>());
}

MaterialAssignCommand::~MaterialAssignCommand()
{
    SafeRelease(entity);
    SafeRelease(newMaterial);
    SafeRelease(oldMaterial);
}

void MaterialAssignCommand::Undo()
{
    materialField.ref.SetValue(oldMaterial);
};

void MaterialAssignCommand::Redo()
{
    materialField.ref.SetValue(newMaterial);
};

Entity* MaterialAssignCommand::GetEntity() const
{
    return entity;
}

NMaterial* MaterialAssignCommand::GetOldMaterial() const
{
    return oldMaterial;
};

NMaterial* MaterialAssignCommand::GetNewMaterial() const
{
    return newMaterial;
};

const Reflection::Field& MaterialAssignCommand::GetMaterialField() const
{
    return materialField;
}

DAVA_VIRTUAL_REFLECTION_IMPL(MaterialAssignCommand)
{
    ReflectionRegistrator<MaterialAssignCommand>::Begin()
    .End();
}
} // namespace DAVA
