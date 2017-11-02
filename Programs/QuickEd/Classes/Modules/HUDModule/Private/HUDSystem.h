#pragma once

#include "EditorSystems/BaseEditorSystem.h"

#include <TArc/DataProcessing/DataWrapper.h>

#include <Math/Vector.h>

namespace DAVA
{
class Any;
class FieldBinder;
}

class ControlContainer;
class FrameControl;
class ControlTransformationSettings;

class HUDSystem : public BaseEditorSystem
{
public:
    HUDSystem(DAVA::ContextAccessor* accessor);
    ~HUDSystem();

    DAVA::Signal<ControlNode*> highlightChanged;
    void HighlightNode(ControlNode* highLight);

    DAVA::Signal<const DAVA::Rect& /*selectionRectControl*/> selectionRectChanged;
    DAVA::Signal<> selectionByRectStarted;
    DAVA::Signal<> selectionByRectFinished;

private:
    enum eSearchOrder
    {
        SEARCH_FORWARD,
        SEARCH_BACKWARD
    };
    struct HUD;

    bool CanProcessInput(DAVA::UIEvent* currentInput) const override;
    void ProcessInput(DAVA::UIEvent* currentInput) override;
    EditorSystemsManager::eDragState RequireNewState(DAVA::UIEvent* currentInput) override;
    void OnDragStateChanged(EditorSystemsManager::eDragState currentState, EditorSystemsManager::eDragState previousState) override;
    void OnDisplayStateChanged(EditorSystemsManager::eDisplayState currentState, EditorSystemsManager::eDisplayState previousState) override;
    CanvasControls CreateCanvasControls() override;
    void DeleteCanvasControls(const CanvasControls& canvasControls) override;

    eSystems GetOrder() const override;
    void OnUpdate() override;

    void SetHighlight(ControlNode* node);

    void OnMagnetLinesChanged(const DAVA::Vector<MagnetLineInfo>& magnetLines);
    void ClearMagnetLines();

    void ProcessCursor(const DAVA::Vector2& pos, eSearchOrder searchOrder = SEARCH_FORWARD);
    HUDAreaInfo GetControlArea(const DAVA::Vector2& pos, eSearchOrder searchOrder) const;
    void SetNewArea(const HUDAreaInfo& HUDAreaInfo);

    void UpdateHUDEnabled();
    SortedControlNodeSet GetSortedControlList() const;

    ControlTransformationSettings* GetSettings();
    DAVA::ContextAccessor* GetAccessor();

    HUDAreaInfo activeAreaInfo;

    DAVA::Vector2 pressedPoint; //corner of selection rect
    DAVA::Vector2 hoveredPoint = DAVA::Vector2(-1.0f, -1.0f);

    DAVA::Map<ControlNode*, std::unique_ptr<HUD>> hudMap;
    std::unique_ptr<FrameControl> selectionRectControl;
    DAVA::Vector<DAVA::RefPtr<DAVA::UIControl>> magnetControls;
    DAVA::Vector<DAVA::RefPtr<DAVA::UIControl>> magnetTargetControls;
    SortedControlNodeSet sortedControlList;
    std::unique_ptr<ControlContainer> hoveredNodeControl;
    std::unique_ptr<DAVA::FieldBinder> fieldBinder;

    DAVA::DataWrapper systemsDataWrapper;
    DAVA::RefPtr<DAVA::UIControl> hudControl;
};
