#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Asset/Asset.h>
#include <Base/FastName.h>
#include <Math/Color.h>
#include <Math/Rect.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class LandscapeProxy;
class SceneEditor2;
class Entity;
class Image;
class Texture;

class ModifyTilemaskCommand : public RECommand
{
public:
    ModifyTilemaskCommand(LandscapeProxy* landscapeProxy, const Rect& updatedRect);
    ~ModifyTilemaskCommand() override;

    void Undo() override;
    void Redo() override;

protected:
    Vector<Image*> undoImageMask;
    Vector<Image*> redoImageMask;
    LandscapeProxy* landscapeProxy = nullptr;
    Rect updatedRect;

    void InvalidateLandscapePart();
    void ApplyImageToTexture(Image* image, const Asset<Texture>& dstTex);

    DAVA_VIRTUAL_REFLECTION(ModifyTilemaskCommand, RECommand);
};
} // namespace DAVA
