#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Render/Material/NMaterial.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class MaterialAddCommand : public RECommand
{
public:
    MaterialAddCommand(NMaterial* material);
    ~MaterialAddCommand();

    void Undo() override{};
    void Redo() override{};

    NMaterial* GetMaterial() const;

protected:
    NMaterial* material = nullptr;

    DAVA_VIRTUAL_REFLECTION(MaterialAddCommand, RECommand);
};

//////////////////////////////////////////////////////////////////////////

class MaterialRemoveCommand : public RECommand
{
public:
    MaterialRemoveCommand(NMaterial* material);
    ~MaterialRemoveCommand();

    void Undo() override;
    void Redo() override;

    NMaterial* GetMaterial() const;

protected:
    NMaterial* material = nullptr;
    NMaterial* parent = nullptr;

    DAVA_VIRTUAL_REFLECTION(MaterialRemoveCommand, RECommand);
};

//////////////////////////////////////////////////////////////////////////

class MaterialSwitchParentCommand : public RECommand
{
public:
    MaterialSwitchParentCommand(NMaterial* instance, NMaterial* newParent);
    ~MaterialSwitchParentCommand();

    void Undo() override;
    void Redo() override;

    NMaterial* GetMaterial() const;

protected:
    NMaterial* oldParent;
    NMaterial* newParent;
    NMaterial* currentInstance;

    DAVA_VIRTUAL_REFLECTION(MaterialSwitchParentCommand, RECommand);
};
} // namespace DAVA
