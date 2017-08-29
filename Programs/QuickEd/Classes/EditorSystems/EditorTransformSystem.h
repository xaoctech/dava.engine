#pragma once

#include "EditorSystems/BaseEditorSystem.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "EditorSystems/SelectionContainer.h"

#include <TArc/DataProcessing/SettingsNode.h>

#include <UI/UIControl.h>
#include <Input/Keyboard.h>
#include <Base/BaseTypes.h>
#include <Math/Vector.h>
#include <Input/InputElements.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class UIGeometricData;
class UIControl;
class Command;
}

class EditorTransformSystem : public BaseEditorSystem
{
public:
    explicit EditorTransformSystem(DAVA::TArc::ContextAccessor* accessor);
    ~EditorTransformSystem() override;

    static DAVA::Vector2 GetMinimumSize();

private:
    enum eDirections
    {
        NEGATIVE_DIRECTION = -1,
        NO_DIRECTION = 0,
        POSITIVE_DIRECTION = 1
    };
    using Directions = DAVA::Array<int, DAVA::Vector2::AXIS_COUNT>;
    using CornersDirections = DAVA::Array<Directions, HUDAreaInfo::CORNERS_COUNT>;
    static const CornersDirections cornersDirections;

    struct MoveInfo;

    EditorSystemsManager::eDragState RequireNewState(DAVA::UIEvent* currentInput) override;
    bool CanProcessInput(DAVA::UIEvent* currentInput) const override;
    void ProcessInput(DAVA::UIEvent* currentInput) override;
    void OnDragStateChanged(EditorSystemsManager::eDragState currentState, EditorSystemsManager::eDragState previousState) override;

    void OnActiveAreaChanged(const HUDAreaInfo& areaInfo);

    void PrepareDrag();

    void ProcessKey(DAVA::eInputElements key);

    void ProcessDrag(const DAVA::Vector2& point);

    void ResizeControl(DAVA::Vector2 delta, bool withPivot, bool rateably);
    DAVA::Vector2 AdjustResizeToMinimumSize(DAVA::Vector2 delta);
    DAVA::Vector2 AdjustResizeToBorderAndToMinimum(DAVA::Vector2 deltaSize, DAVA::Vector2 transformPoint, Directions directions);
    DAVA::Vector2 AdjustResizeToBorder(DAVA::Vector2 deltaSize, DAVA::Vector2 transformPoint, Directions directions, DAVA::Vector<MagnetLineInfo>& magnets);

    void MovePivot(DAVA::Vector2 delta);
    DAVA::Vector2 AdjustPivotToNearestArea(DAVA::Vector2& delta);

    bool RotateControl(const DAVA::Vector2& pos);
    DAVA::float32 AdjustRotateToFixedAngle(DAVA::float32 deltaAngle, DAVA::float32 originalAngle);

    void MoveAllSelectedControlsByMouse(DAVA::Vector2 delta, bool canAdjust);
    void MoveAllSelectedControlsByKeyboard(DAVA::Vector2 delta);
    DAVA::Vector2 AdjustMoveToNearestBorder(DAVA::Vector2 delta, DAVA::Vector<MagnetLineInfo>& magnetLines, const DAVA::UIGeometricData* parentGD, const DAVA::UIControl* control);

    void CorrectNodesToMove();
    void UpdateNeighboursToMove();

    void ClampAngle();
    struct MagnetLine;
    DAVA::Vector<MagnetLine> CreateMagnetLines(const DAVA::Rect& box, const DAVA::UIGeometricData* parentGD, const DAVA::Vector<DAVA::UIControl*>& neighbours, DAVA::Vector2::eAxis axis);
    void CreateMagnetLinesToParent(const DAVA::Rect& box, const DAVA::UIGeometricData* parentGD, DAVA::Vector2::eAxis axis, DAVA::Vector<MagnetLine>& lines);
    void CreateMagnetLinesToNeghbours(const DAVA::Rect& box, const DAVA::Vector<DAVA::UIControl*>& neighbours, DAVA::Vector2::eAxis axis, DAVA::Vector<MagnetLine>& lines);
    void CreateMagnetLinesToGuides(const DAVA::Rect& box, const DAVA::UIGeometricData* parentGD, DAVA::Vector2::eAxis axis, DAVA::Vector<MagnetLine>& lines);

    void ExtractMatchedLines(DAVA::Vector<MagnetLineInfo>& magnets, const DAVA::Vector<MagnetLine>& magnetLines, const DAVA::UIControl* control, DAVA::Vector2::eAxis axis);
    bool IsShiftPressed() const;

    bool CanMagnet() const;

    HUDAreaInfo::eArea activeArea = HUDAreaInfo::NO_AREA;
    ControlNode* activeControlNode = nullptr;
    DAVA::Vector2 extraDelta;
    DAVA::Map<const ControlNode*, DAVA::Vector2> extraDeltaToMoveControls;
    //this variable is used for rotation only
    DAVA::Vector2 previousMousePos;
    SelectedControls selectedControlNodes;
    DAVA::List<std::unique_ptr<MoveInfo>> nodesToMoveInfos;
    DAVA::Vector<DAVA::UIControl*> neighbours;

    DAVA::UIGeometricData parentGeometricData;
    DAVA::UIGeometricData controlGeometricData;
    AbstractProperty* sizeProperty = nullptr;
    AbstractProperty* positionProperty = nullptr;
    AbstractProperty* angleProperty = nullptr;
    AbstractProperty* pivotProperty = nullptr;
};
