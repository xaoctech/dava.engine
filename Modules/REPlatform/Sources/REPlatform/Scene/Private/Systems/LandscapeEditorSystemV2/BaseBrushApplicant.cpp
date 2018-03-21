#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseBrushApplicant.h"

namespace DAVA
{
void BaseBrushApplicant::SetCursorUV(const Vector4& uv)
{
    cursorUV = uv;
}

void BaseBrushApplicant::SetRotation(const Vector2& rotation)
{
    cursorRotarion = rotation;
}

void BaseBrushApplicant::SetInvertionFactor(float32 invertionFactor_)
{
    invertionFactor = invertionFactor_;
}

void BaseBrushApplicant::SetCursorTexture(Asset<Texture> texture)
{
    cursorTexture = texture;
}
} // namespace DAVA