#pragma once

#include "REPlatform/Commands/RECommand.h"
#include "REPlatform/DataNodes/Selectable.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class TransformCommand : public RECommand
{
public:
    TransformCommand(Selectable object, const Matrix4& origTransform, const Matrix4& newTransform);

    void Undo() override;
    void Redo() override;

    Entity* GetEntity() const;
    const Selectable& GetTransformedObject() const;

protected:
    Selectable object;
    Matrix4 undoTransform;
    Matrix4 redoTransform;

    DAVA_VIRTUAL_REFLECTION(TransformCommand, RECommand);
};
} // namespace DAVA
