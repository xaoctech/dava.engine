#include "Classes/Modules/HUDModule/Private/HUDSystem.h"
#include "Classes/Modules/DocumentsModule/EditorSystemsData.h"

#include "Classes/Modules/HUDModule/Private/HUDControls.h"

#include "Modules/DocumentsModule/DocumentData.h"

#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/VisibleValueProperty.h"

#include "Classes/EditorSystems/ControlTransformationSettings.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/Core/FieldBinder.h>
#include <TArc/DataProcessing/DataContext.h>
#include <TArc/Utils/Utils.h>

#include <Base/BaseTypes.h>
#include <UI/UIControl.h>
#include <UI/UIEvent.h>

using namespace DAVA;

namespace
{
const Array<HUDAreaInfo::eArea, 2> AreasToHide = { { HUDAreaInfo::PIVOT_POINT_AREA, HUDAreaInfo::ROTATE_AREA } };
}

std::unique_ptr<ControlContainer> CreateControlContainer(HUDAreaInfo::eArea area, DAVA::TArc::ContextAccessor* accessor)
{
    switch (area)
    {
    case HUDAreaInfo::PIVOT_POINT_AREA:
        return std::unique_ptr<ControlContainer>(new PivotPointControl(accessor));
    case HUDAreaInfo::ROTATE_AREA:
        return std::unique_ptr<ControlContainer>(new RotateControl(accessor));
    case HUDAreaInfo::TOP_LEFT_AREA:
    case HUDAreaInfo::TOP_CENTER_AREA:
    case HUDAreaInfo::TOP_RIGHT_AREA:
    case HUDAreaInfo::CENTER_LEFT_AREA:
    case HUDAreaInfo::CENTER_RIGHT_AREA:
    case HUDAreaInfo::BOTTOM_LEFT_AREA:
    case HUDAreaInfo::BOTTOM_CENTER_AREA:
    case HUDAreaInfo::BOTTOM_RIGHT_AREA:
        return std::unique_ptr<ControlContainer>(new FrameRectControl(area, accessor));
    case HUDAreaInfo::FRAME_AREA:
        return std::unique_ptr<ControlContainer>(new FrameControl(FrameControl::SELECTION, accessor));
    default:
        DVASSERT(!"unacceptable value of area");
        return std::unique_ptr<ControlContainer>(nullptr);
    }
}

struct HUDSystem::HUD
{
    HUD(ControlNode* node, HUDSystem* hudSystem, UIControl* hudControl);
    ~HUD();
    ControlNode* node = nullptr;
    UIControl* control = nullptr;
    UIControl* hudControl = nullptr;
    std::unique_ptr<ControlContainer> container;
    Map<HUDAreaInfo::eArea, ControlContainer*> hudControls;
};

HUDSystem::HUD::HUD(ControlNode* node_, HUDSystem* hudSystem, UIControl* hudControl_)
    : node(node_)
    , control(node_->GetControl())
    , hudControl(hudControl_)
    , container(new HUDContainer(node_))
{
    container->SetName(String("Container_for_HUD_controls_of_node"));
    DAVA::Vector<HUDAreaInfo::eArea> areas;
    if (node->GetParent() != nullptr && node->GetParent()->GetControl() != nullptr)
    {
        areas.reserve(HUDAreaInfo::AREAS_COUNT);
        for (int area = HUDAreaInfo::AREAS_COUNT - 1; area >= HUDAreaInfo::AREAS_BEGIN; --area)
        {
            ControlTransformationSettings* settings = hudSystem->GetSettings();
            if ((settings->showPivot == false && area == HUDAreaInfo::PIVOT_POINT_AREA) ||
                (settings->showRotate == false && area == HUDAreaInfo::ROTATE_AREA))
            {
                continue;
            }
            areas.push_back(static_cast<HUDAreaInfo::eArea>(area));
        }
    }
    else
    {
        //custom areas for root control
        areas = {
            HUDAreaInfo::TOP_LEFT_AREA,
            HUDAreaInfo::TOP_CENTER_AREA,
            HUDAreaInfo::TOP_RIGHT_AREA,
            HUDAreaInfo::CENTER_LEFT_AREA,
            HUDAreaInfo::CENTER_RIGHT_AREA,
            HUDAreaInfo::BOTTOM_LEFT_AREA,
            HUDAreaInfo::BOTTOM_CENTER_AREA,
            HUDAreaInfo::BOTTOM_RIGHT_AREA
        };
    }
    for (HUDAreaInfo::eArea area : areas)
    {
        std::unique_ptr<ControlContainer> controlContainer = CreateControlContainer(area, hudSystem->GetAccessor());
        hudControls[area] = controlContainer.get();
        container->AddChild(std::move(controlContainer));
    }
    container->AddToParent(hudControl);
    container->InitFromGD(control->GetGeometricData());
}

HUDSystem::HUD::~HUD()
{
    container->RemoveFromParent(hudControl);
}

class HUDControl : public UIControl
{
    void Draw(const UIGeometricData& geometricData) override
    {
        UpdateLayout();
        UIControl::Draw(geometricData);
    }
};

HUDSystem::HUDSystem(DAVA::TArc::ContextAccessor* accessor)
    : BaseEditorSystem(accessor)
{
    systemsDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<EditorSystemsData>());
    GetSystemsManager()->magnetLinesChanged.Connect(this, &HUDSystem::OnMagnetLinesChanged);
}

HUDSystem::~HUDSystem() = default;

BaseEditorSystem::eSystems HUDSystem::GetOrder() const
{
    return eSystems::HUD;
}

void HUDSystem::OnUpdate()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    DataContext* activeContext = accessor->GetActiveContext();
    if (activeContext == nullptr)
    {
        hudMap.clear();
        SetNewArea(HUDAreaInfo());
        OnHighlightNode(nullptr);
        return;
    }

    DocumentData* documentData = activeContext->GetData<DocumentData>();
    SelectedNodes selection = documentData->GetSelectedNodes();
    for (auto iter = hudMap.begin(); iter != hudMap.end();)
    {
        ControlNode* node = iter->first;
        if (selection.find(node) == selection.end())
        {
            iter = hudMap.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    for (PackageBaseNode* node : selection)
    {
        ControlNode* controlNode = dynamic_cast<ControlNode*>(node);
        if (controlNode != nullptr)
        {
            if (hudMap.find(controlNode) == hudMap.end())
            {
                hudMap[controlNode] = std::make_unique<HUD>(controlNode, this, hudControl.Get());
            }
        }
    }

    //show Rotate and Pivot only if only one control is selected
    bool showAreas = hudMap.size() == 1;

    for (const auto& iter : hudMap)
    {
        for (HUDAreaInfo::eArea area : AreasToHide)
        {
            auto hudControlsIter = iter.second->hudControls.find(area);
            if (hudControlsIter != iter.second->hudControls.end())
            {
                ControlContainer* controlContainer = hudControlsIter->second;
                controlContainer->SetSystemVisible(showAreas);
            }
        }
    }

    for (const auto& hudPair : hudMap)
    {
        const UIGeometricData& controlGD = hudPair.first->GetControl()->GetGeometricData();
        hudPair.second->container->InitFromGD(controlGD);
    }

    EditorSystemsData* systemsData = accessor->GetGlobalContext()->GetData<EditorSystemsData>();
    if (GetSystemsManager()->GetDragState() == EditorSystemsManager::NoDrag &&
        systemsData->IsHighlightDisabled() == false)
    {
        ControlNode* node = GetSystemsManager()->GetControlNodeAtPoint(hoveredPoint);
        SetHighlight(node);
    }
    else
    {
        SetHighlight(nullptr);
    }

    if (GetSystemsManager()->GetDragState() == EditorSystemsManager::NoDrag)
    {
        bool findPivot = hudMap.size() == 1 && IsKeyPressed(eModifierKeys::CONTROL) && IsKeyPressed(eModifierKeys::ALT);
        eSearchOrder searchOrder = findPivot ? SEARCH_BACKWARD : SEARCH_FORWARD;
        ProcessCursor(hoveredPoint, searchOrder);
    }
}

void HUDSystem::ProcessInput(UIEvent* currentInput)
{
    UIEvent::Phase phase = currentInput->phase;
    if (currentInput->point.x > 0.0f && currentInput->point.y > 0.0f)
    {
        hoveredPoint = currentInput->point;
    }

    switch (phase)
    {
    case UIEvent::Phase::DRAG:
        if (GetSystemsManager()->GetDragState() == EditorSystemsManager::SelectByRect)
        {
            Vector2 point(pressedPoint);
            Vector2 size(hoveredPoint - pressedPoint);
            if (size.x < 0.0f)
            {
                point.x += size.x;
                size.x *= -1.0f;
            }
            if (size.y <= 0.0f)
            {
                point.y += size.y;
                size.y *= -1.0f;
            }

            selectionRectControl->SetRect(Rect(point, size));
            GetSystemsManager()->selectionRectChanged.Emit(selectionRectControl->GetAbsoluteRect());
        }
        break;
    default:
        break;
    }
}

void HUDSystem::SetHighlight(ControlNode* node)
{
    highlightChanged.Emit(node);
    HighlightNode(node);
}

void HUDSystem::HighlightNode(ControlNode* node)
{
    using namespace DAVA::TArc;

    if (hoveredNodeControl != nullptr && node != nullptr && hoveredNodeControl->IsDrawableControl(node->GetControl()))
    {
        return;
    }
    if (hoveredNodeControl != nullptr)
    {
        hoveredNodeControl->RemoveFromParent(hudControl.Get());
        hoveredNodeControl = nullptr;
    }

    if (node != nullptr)
    {
        DataContext* activeContext = accessor->GetActiveContext();
        DVASSERT(activeContext != nullptr);
        DocumentData* documentData = activeContext->GetData<DocumentData>();
        const SelectedNodes& selectedNodes = documentData->GetSelectedNodes();

        if (selectedNodes.find(node) != selectedNodes.end())
        {
            return;
        }

        UIControl* targetControl = node->GetControl();
        DVASSERT(hoveredNodeControl == nullptr);
        hoveredNodeControl = CreateHighlightRect(node, GetAccessor());
        hoveredNodeControl->AddToParent(hudControl.Get());
    }
}

void HUDSystem::OnMagnetLinesChanged(const Vector<MagnetLineInfo>& magnetLines)
{
    static const float32 extraSizeValue = 50.0f;
    DVASSERT(magnetControls.size() == magnetTargetControls.size());

    const size_type magnetsSize = magnetControls.size();
    const size_type newMagnetsSize = magnetLines.size();
    if (newMagnetsSize < magnetsSize)
    {
        auto linesRIter = magnetControls.rbegin();
        auto rectsRIter = magnetTargetControls.rbegin();
        size_type count = magnetsSize - newMagnetsSize;
        for (size_type i = 0; i < count; ++i)
        {
            UIControl* lineControl = (*linesRIter++).Get();
            UIControl* targetRectControl = (*rectsRIter++).Get();
            hudControl->RemoveControl(lineControl);
            hudControl->RemoveControl(targetRectControl);
        }
        const auto& linesEnd = magnetControls.end();
        const auto& targetRectsEnd = magnetTargetControls.end();
        magnetControls.erase(linesEnd - count, linesEnd);
        magnetTargetControls.erase(targetRectsEnd - count, targetRectsEnd);
    }
    else if (newMagnetsSize > magnetsSize)
    {
        size_type count = newMagnetsSize - magnetsSize;

        magnetControls.reserve(count);
        magnetTargetControls.reserve(count);
        for (size_type i = 0; i < count; ++i)
        {
            RefPtr<UIControl> lineControl(new UIControl());
            lineControl->SetName(FastName("magnet_line_control"));
            ::SetupHUDMagnetLineControl(lineControl.Get(), accessor);
            hudControl->AddControl(lineControl.Get());
            magnetControls.emplace_back(lineControl);

            RefPtr<UIControl> rectControl(new UIControl());
            rectControl->SetName(FastName("rect_of_target_control_which_we_magnet_to"));
            ::SetupHUDMagnetRectControl(rectControl.Get(), accessor);
            hudControl->AddControl(rectControl.Get());
            magnetTargetControls.emplace_back(rectControl);
        }
    }
    DVASSERT(magnetLines.size() == magnetControls.size() && magnetControls.size() == magnetTargetControls.size());
    for (size_t i = 0, size = magnetLines.size(); i < size; ++i)
    {
        const MagnetLineInfo& line = magnetLines.at(i);
        const auto& gd = line.gd;
        auto linePos = line.rect.GetPosition();
        auto lineSize = line.rect.GetSize();

        linePos = ::Rotate(linePos, gd->angle);
        linePos *= gd->scale;
        lineSize[line.axis] *= gd->scale[line.axis];
        Vector2 gdPos = gd->position - ::Rotate(gd->pivotPoint * gd->scale, gd->angle);

        UIControl* lineControl = magnetControls.at(i).Get();
        float32 angle = line.gd->angle;
        Vector2 extraSize(line.axis == Vector2::AXIS_X ? extraSizeValue : 0.0f, line.axis == Vector2::AXIS_Y ? extraSizeValue : 0.0f);
        Vector2 extraPos = ::Rotate(extraSize, angle) / 2.0f;
        Rect lineRect(Vector2(linePos + gdPos) - extraPos, lineSize + extraSize);
        lineControl->SetRect(lineRect);
        lineControl->SetAngle(angle);

        linePos = line.targetRect.GetPosition();
        lineSize = line.targetRect.GetSize();

        linePos = ::Rotate(linePos, gd->angle);
        linePos *= gd->scale;
        lineSize *= gd->scale;

        UIControl* rectControl = magnetTargetControls.at(i).Get();
        rectControl->SetRect(Rect(linePos + gdPos, lineSize));
        rectControl->SetAngle(line.gd->angle);
    }
}

void HUDSystem::ProcessCursor(const Vector2& pos, eSearchOrder searchOrder)
{
    if (GetSystemsManager()->GetDragState() == EditorSystemsManager::SelectByRect)
    {
        return;
    }
    SetNewArea(GetControlArea(pos, searchOrder));
}

HUDAreaInfo HUDSystem::GetControlArea(const Vector2& pos, eSearchOrder searchOrder) const
{
    uint32 end = HUDAreaInfo::AREAS_BEGIN;
    int sign = 1;
    if (searchOrder == SEARCH_BACKWARD)
    {
        if (HUDAreaInfo::AREAS_BEGIN != HUDAreaInfo::AREAS_COUNT)
        {
            end = HUDAreaInfo::AREAS_COUNT - 1;
        }
        sign = -1;
    }
    for (uint32 i = HUDAreaInfo::AREAS_BEGIN; i < HUDAreaInfo::AREAS_COUNT; ++i)
    {
        for (const auto& iter : GetSortedControlList())
        {
            ControlNode* node = dynamic_cast<ControlNode*>(iter);
            DVASSERT(nullptr != node);
            auto findIter = hudMap.find(node);
            DVASSERT(findIter != hudMap.end(), "hud map corrupted");
            const auto& hud = findIter->second;
            if (hud->container->GetVisibilityFlag() && !hud->container->IsHiddenForDebug())
            {
                HUDAreaInfo::eArea area = static_cast<HUDAreaInfo::eArea>(end + sign * i);
                auto hudControlsIter = hud->hudControls.find(area);
                if (hudControlsIter != hud->hudControls.end())
                {
                    const auto& controlContainer = hudControlsIter->second;
                    if (controlContainer->GetVisibilityFlag() && !controlContainer->IsHiddenForDebug() && controlContainer->IsPointInside(pos))
                    {
                        return HUDAreaInfo(hud->node, area);
                    }
                }
            }
        }
    }
    return HUDAreaInfo();
}

void HUDSystem::SetNewArea(const HUDAreaInfo& areaInfo)
{
    if (activeAreaInfo.area != areaInfo.area || activeAreaInfo.owner != areaInfo.owner)
    {
        activeAreaInfo = areaInfo;
        GetSystemsManager()->activeAreaChanged.Emit(activeAreaInfo);
    }
}

void HUDSystem::OnDragStateChanged(EditorSystemsManager::eDragState currentState, EditorSystemsManager::eDragState previousState)
{
    switch (currentState)
    {
    case EditorSystemsManager::SelectByRect:
        DVASSERT(selectionRectControl == nullptr);
        selectionRectControl.reset(new FrameControl(FrameControl::SELECTION_RECT, accessor));
        selectionRectControl->AddToParent(hudControl.Get());
        break;
    case EditorSystemsManager::DragScreen:
        UpdateHUDEnabled();
        break;
    default:
        break;
    }

    switch (previousState)
    {
    case EditorSystemsManager::SelectByRect:
        DVASSERT(selectionRectControl != nullptr);
        selectionRectControl->RemoveFromParent(hudControl.Get());
        selectionRectControl = nullptr;
        break;
    case EditorSystemsManager::Transform:
        ClearMagnetLines();
        break;
    case EditorSystemsManager::DragScreen:
        UpdateHUDEnabled();
        break;
    default:
        break;
    }
}

void HUDSystem::OnDisplayStateChanged(EditorSystemsManager::eDisplayState, EditorSystemsManager::eDisplayState)
{
    UpdateHUDEnabled();
}

CanvasControls HUDSystem::CreateCanvasControls()
{
    DVASSERT(hudControl.Valid() == false);

    hudControl.Set(new DAVA::UIControl());
    hudControl->SetName("hud_control");

    return { { hudControl } };
}

void HUDSystem::DeleteCanvasControls(const CanvasControls& canvasControls)
{
    hudMap.clear();
    hudControl = nullptr;
}

bool HUDSystem::CanProcessInput(DAVA::UIEvent* currentInput) const
{
    using namespace DAVA::TArc;

    DataContext* activeContext = accessor->GetActiveContext();
    if (hudControl->IsVisible() == false || activeContext == nullptr)
    {
        return false;
    }
    EditorSystemsManager::eDisplayState displayState = GetSystemsManager()->GetDisplayState();
    EditorSystemsManager::eDragState dragState = GetSystemsManager()->GetDragState();
    return (displayState == EditorSystemsManager::Edit || displayState == EditorSystemsManager::Preview)
    && dragState != EditorSystemsManager::Transform;
}

EditorSystemsManager::eDragState HUDSystem::RequireNewState(DAVA::UIEvent* currentInput)
{
    EditorSystemsManager::eDragState dragState = GetSystemsManager()->GetDragState();
    //ignore all input devices except mouse while selecting by rect
    if (dragState == EditorSystemsManager::SelectByRect && currentInput->device != eInputDevices::MOUSE)
    {
        return EditorSystemsManager::SelectByRect;
    }
    if (dragState == EditorSystemsManager::Transform || dragState == EditorSystemsManager::DragScreen)
    {
        return EditorSystemsManager::NoDrag;
    }

    Vector2 point = currentInput->point;
    if (currentInput->phase == UIEvent::Phase::BEGAN
        && dragState != EditorSystemsManager::SelectByRect)
    {
        pressedPoint = point;
    }
    if (currentInput->phase == UIEvent::Phase::DRAG)
    {
        //if we in selectByRect and still drag mouse - continue this state
        if (dragState == EditorSystemsManager::SelectByRect)
        {
            return EditorSystemsManager::SelectByRect;
        }
        //check that we can draw rect
        Vector<ControlNode*> nodesUnderPoint;
        auto predicate = [point](const ControlNode* node) -> bool {
            const auto visibleProp = node->GetRootProperty()->GetVisibleProperty();
            DVASSERT(node->GetControl() != nullptr);
            return visibleProp->GetVisibleInEditor() && node->GetControl()->IsPointInside(point);
        };
        GetSystemsManager()->CollectControlNodes(std::back_inserter(nodesUnderPoint), predicate);
        bool noHudableControls = nodesUnderPoint.empty() || (nodesUnderPoint.size() == 1 && nodesUnderPoint.front()->GetParent()->GetControl() == nullptr);

        if (noHudableControls)
        {
            //distinguish between mouse click and mouse drag sometimes is less than few pixels
            //so lets select by rect only if we sure that is not mouse click
            Vector2 rectSize(pressedPoint - point);
            ControlTransformationSettings* settings = GetSettings();
            if (fabs(rectSize.dx) >= settings->minimumSelectionRectSize.dx || fabs(rectSize.dy) >= settings->minimumSelectionRectSize.dy)
            {
                return EditorSystemsManager::SelectByRect;
            }
        }
    }
    return EditorSystemsManager::NoDrag;
}

void HUDSystem::ClearMagnetLines()
{
    static const Vector<MagnetLineInfo> emptyVector;
    OnMagnetLinesChanged(emptyVector);
}

void HUDSystem::UpdateHUDEnabled()
{
    bool enabled = GetSystemsManager()->GetDragState() != EditorSystemsManager::DragScreen
    && GetSystemsManager()->GetDisplayState() == EditorSystemsManager::Edit;
    hudControl->SetVisibilityFlag(enabled);
}

ControlTransformationSettings* HUDSystem::GetSettings()
{
    return accessor->GetGlobalContext()->GetData<ControlTransformationSettings>();
}

DAVA::TArc::ContextAccessor* HUDSystem::GetAccessor()
{
    return accessor;
}

SortedControlNodeSet HUDSystem::GetSortedControlList() const
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    SortedControlNodeSet sortedControls(CompareByLCA);

    DataContext* activeContext = accessor->GetActiveContext();
    if (activeContext == nullptr)
    {
        return sortedControls;
    }

    DocumentData* documentData = activeContext->GetData<DocumentData>();
    const SelectedNodes& selectedNodes = documentData->GetSelectedNodes();

    for (PackageBaseNode* node : selectedNodes)
    {
        ControlNode* controlNode = dynamic_cast<ControlNode*>(node);
        if (nullptr != controlNode && nullptr != controlNode->GetControl())
        {
            sortedControls.insert(controlNode);
        }
    }
    return sortedControls;
}
