#pragma once

#include "EditorSystems/BaseEditorSystem.h"
#include "UI/Preview/Data/CanvasDataAdapter.h"

#include <TArc/DataProcessing/DataWrapper.h>

namespace DAVA
{
class Vector2;
class UIControl;
}

class EditorCanvas final : public BaseEditorSystem
{
public:
    EditorCanvas(EditorSystemsManager* parent, DAVA::TArc::ContextAccessor* accessor);

    bool CanProcessInput(DAVA::UIEvent* currentInput) const override;
    EditorSystemsManager::eDragState RequireNewState(DAVA::UIEvent* currentInput) override;
    void ProcessInput(DAVA::UIEvent* currentInput) override;

private:
    void InitFieldBinder();
    DAVA::float32 GetScaleFromWheelEvent(DAVA::int32 ticksCount) const;
    void OnMovableControlPositionChanged(const DAVA::Any& movableControlPosition);
    void OnScaleChanged(const DAVA::Any& scale);

    void UpdateMovableControlState();

    CanvasDataAdapter canvasDataAdapter;
    DAVA::TArc::DataWrapper canvasDataAdapterWrapper;

    bool isMouseMidButtonPressed = false;
    bool isSpacePressed = false;
};
