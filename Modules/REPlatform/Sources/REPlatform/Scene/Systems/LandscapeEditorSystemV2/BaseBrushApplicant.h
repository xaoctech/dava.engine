#pragma once

#include <Base/RefPtr.h>
#include <Math/Vector.h>
#include <Math/Rect.h>
#include <Render/Texture.h>

namespace DAVA
{
class Scene;
class Command;
class RECommandNotificationObject;
class BaseBrushApplicant
{
public:
    virtual ~BaseBrushApplicant() = default;

    void SetCursorUV(const Vector4& uv);
    void SetRotation(const Vector2& rotation);
    void SetInvertionFactor(float32 invertionFactor);
    void SetCursorTexture(Asset<Texture> texture);

    virtual void StoreSnapshots() = 0;
    virtual void ApplyBrush(Scene* scene, const Rect& applyRect) = 0;
    virtual std::unique_ptr<Command> CreateDiffCommand(const Rect& operationRect) = 0;

    virtual void OnCommandExecuted(const RECommandNotificationObject& notif)
    {
    }

protected:
    Vector4 cursorUV;
    Vector2 cursorRotarion;
    float32 invertionFactor = 0.0f;
    Asset<Texture> cursorTexture;
};
} // namespace DAVA