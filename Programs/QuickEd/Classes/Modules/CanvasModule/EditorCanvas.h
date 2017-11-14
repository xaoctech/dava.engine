#pragma once

#include "EditorSystems/BaseEditorSystem.h"
#include "Modules/CanvasModule/CanvasDataAdapter.h"

#include <TArc/DataProcessing/DataWrapper.h>

namespace DAVA
{
class Vector2;
class UIControl;
}

class EditorCanvas final : public BaseEditorSystem
{
public:
    EditorCanvas(DAVA::TArc::ContextAccessor* accessor);

private:
    bool CanProcessInput(DAVA::UIEvent* currentInput, bool generated) const override;
    EditorSystemsManager::eDragState RequireNewState(DAVA::UIEvent* currentInput) override;
    void ProcessInput(DAVA::UIEvent* currentInput, bool generated) override;
    CanvasControls CreateCanvasControls() override;
    void DeleteCanvasControls(const CanvasControls& canvasControls) override;
    eSystems GetOrder() const override;
    void OnUpdate() override;

    void OnPositionChanged(const DAVA::Any& movableControlPosition);
    void OnScaleChanged(const DAVA::Any& scale);
    void MoveSceneByUpdate();

    CanvasDataAdapter canvasDataAdapter;
    DAVA::TArc::DataWrapper canvasDataAdapterWrapper;
    bool isMouseMidButtonPressed = false;
    bool isSpacePressed = false;
};
