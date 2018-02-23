#pragma once

#include <Math/Vector.h>
#include <Engine/EngineTypes.h>
#include <Input/InputElements.h>

namespace DAVA
{
class UIEvent;
class Scene;
class BrushInputController
{
public:
    virtual ~BrushInputController() = default;

    virtual void OnInput(UIEvent* e) = 0;

    const Vector2& GetCursorPos() const;
    const Vector4& GetBeginOperationCursorUV() const;
    virtual const Vector4& GetCurrentCursorUV() const;

    void BeginOperation(const Vector4& cursorUV);
    bool IsInOperation() const;
    void UpdateCurrentCursorUV(const Vector4& cursorUV);
    void EndOperation();

    void Init(Scene* scene);
    void Reset();

    bool IsModifierPressed(eModifierKeys modifier) const;
    bool IsKeyPressed(eInputElements element) const;

protected:
    void UpdateCursorPos(const Vector2& pos);

private:
    Vector2 cursorPos = Vector2(0.0, 0.0);
    Vector4 beginOperationCursorUV = Vector4(0.0, 0.0, 0.0, 1.0);
    Vector4 currentCursorUV = Vector4(0.0, 0.0, 0.0, 1.0);

    Scene* scene = nullptr;
};

class DefaultBrushInputController : public BrushInputController
{
public:
    void OnInput(UIEvent* e) override;
};
} // namespace DAVA
