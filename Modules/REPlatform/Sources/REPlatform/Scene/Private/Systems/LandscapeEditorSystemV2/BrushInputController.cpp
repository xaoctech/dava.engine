#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushInputController.h"
#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/LandscapeEditorSystemV2.h"

#include <TArc/Utils/Utils.h>

#include <DeviceManager/DeviceManager.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Engine/EngineTypes.h>
#include <Input/Keyboard.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <Render/Highlevel/RenderSystem.h>
#include <UI/UIControlSystem.h>

namespace DAVA
{
const Vector2& BrushInputController::GetCursorPos() const
{
    return cursorPos;
}

const Vector4& BrushInputController::GetBeginOperationCursorUV() const
{
    return beginOperationCursorUV;
}

const Vector4& BrushInputController::GetCurrentCursorUV() const
{
    return currentCursorUV;
}

void BrushInputController::BeginOperation(const Vector4& cursorUV)
{
    beginOperationCursorUV = cursorUV;
    currentCursorUV = beginOperationCursorUV;
}

void BrushInputController::UpdateCurrentCursorUV(const Vector4& cursorUV)
{
    currentCursorUV = cursorUV;
}

void BrushInputController::EndOperation()
{
    beginOperationCursorUV = Vector4(0.0, 0.0, 0.0, 1.0);
    currentCursorUV = beginOperationCursorUV;
}

void BrushInputController::Init(Scene* scene_)
{
    scene = scene_;
}

void BrushInputController::Reset()
{
    scene = nullptr;
}

bool BrushInputController::IsModifierPressed(eModifierKeys modifier) const
{
    return DAVA::IsKeyPressed(modifier);
}

bool BrushInputController::IsKeyPressed(eInputElements element) const
{
    const Keyboard* keyboard = GetEngineContext()->deviceManager->GetKeyboard();
    if (keyboard == nullptr)
    {
        return false;
    }

    return keyboard->GetKeyState(element).IsPressed();
}

void BrushInputController::UpdateCursorPos(const Vector2& pos)
{
    DVASSERT(scene != nullptr);
    VirtualCoordinatesSystem* vcs = GetEngineContext()->uiControlSystem->vcs;
    RenderSystem* renderSystem = scene->GetRenderSystem();
    Rect viewport = renderSystem->GetConfiguration().viewport;

    Vector2 lastCursorPosInScreenSpace = vcs->ConvertVirtualToPhysical(pos);
    lastCursorPosInScreenSpace.x /= viewport.dx;
    lastCursorPosInScreenSpace.y /= viewport.dy;

    cursorPos = lastCursorPosInScreenSpace;
}

bool BrushInputController::IsInOperation() const
{
    return beginOperationCursorUV.w < 1.0;
}

void DefaultBrushInputController::OnInput(UIEvent* e)
{
    if (e->phase == UIEvent::Phase::MOVE ||
        e->phase == UIEvent::Phase::DRAG)
    {
        UpdateCursorPos(e->point);
    }
}
} // namespace DAVA
