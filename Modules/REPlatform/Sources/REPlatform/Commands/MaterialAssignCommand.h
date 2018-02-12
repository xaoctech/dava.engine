#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Render/Material/NMaterial.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class MaterialAssignCommand : public RECommand
{
public:
    MaterialAssignCommand(Entity* entity, NMaterial* material, const Reflection::Field& materialField);
    ~MaterialAssignCommand();

    void Undo() override;
    void Redo() override;

    Entity* GetEntity() const;
    NMaterial* GetOldMaterial() const;
    NMaterial* GetNewMaterial() const;
    const Reflection::Field& GetMaterialField() const;

protected:
    Entity* entity = nullptr;
    NMaterial* oldMaterial = nullptr;
    NMaterial* newMaterial = nullptr;
    Reflection::Field materialField;

    DAVA_VIRTUAL_REFLECTION(MaterialAssignCommand, RECommand);
};
} // namespace DAVA
