#pragma once

#include "EditorSystems/BaseEditorSystem.h"

#include <Math/Vector.h>

namespace DAVA
{
class Any;
namespace TArc
{
class FieldBinder;
}
}

class ControlContainer;
class FrameControl;
class ControlTransformationSettings;

class HUDSystem : public BaseEditorSystem
{
public:
    HUDSystem(EditorSystemsManager* parent, DAVA::TArc::ContextAccessor* accessor);
    ~HUDSystem() override;

private:
    enum eSearchOrder
    {
        SEARCH_FORWARD,
        SEARCH_BACKWARD
    };
    struct HUD;

    void InitFieldBinder();
    bool CanProcessInput(DAVA::UIEvent* currentInput) const override;
    void ProcessInput(DAVA::UIEvent* currentInput) override;
    EditorSystemsManager::eDragState RequireNewState(DAVA::UIEvent* currentInput) override;
    void OnDragStateChanged(EditorSystemsManager::eDragState currentState, EditorSystemsManager::eDragState previousState) override;
    void OnDisplayStateChanged(EditorSystemsManager::eDisplayState currentState, EditorSystemsManager::eDisplayState previousState) override;

    void OnSelectionChanged(const DAVA::Any& selection);
    void OnHighlightNode(ControlNode* node);

    void OnMagnetLinesChanged(const DAVA::Vector<MagnetLineInfo>& magnetLines);
    void ClearMagnetLines();

    void ProcessCursor(const DAVA::Vector2& pos, eSearchOrder searchOrder = SEARCH_FORWARD);
    HUDAreaInfo GetControlArea(const DAVA::Vector2& pos, eSearchOrder searchOrder) const;
    void SetNewArea(const HUDAreaInfo& HUDAreaInfo);

    void UpdateAreasVisibility();
    void UpdateHUDEnabled();
    void OnUpdate();

    ControlTransformationSettings* GetSettings();
    DAVA::TArc::ContextAccessor* GetAccessor();

    HUDAreaInfo activeAreaInfo;

    DAVA::Vector2 pressedPoint; //corner of selection rect
    DAVA::Vector2 hoveredPoint;

    DAVA::Map<ControlNode*, std::unique_ptr<HUD>> hudMap;
    std::unique_ptr<FrameControl> selectionRectControl;
    DAVA::Vector<DAVA::RefPtr<DAVA::UIControl>> magnetControls;
    DAVA::Vector<DAVA::RefPtr<DAVA::UIControl>> magnetTargetControls;
    SortedControlNodeSet sortedControlList;
    std::unique_ptr<ControlContainer> hoveredNodeControl;
    std::unique_ptr<DAVA::TArc::FieldBinder> fieldBinder;
};
