#include "Modules/CanvasModule/EditorControlsView.h"

#include "EditorSystems/EditorSystemsManager.h"

#include "Model/PackageHierarchy/PackageBaseNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/ControlProperties/RootProperty.h"

#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/CanvasModule/CanvasData.h"
#include "UI/Preview/PreviewWidgetSettings.h"

#include <TArc/Core/FieldBinder.h>
#include <TArc/DataProcessing/DataListener.h>

#include <Render/2D/Sprite.h>
#include <Render/2D/Systems/RenderSystem2D.h>
#include <UI/UIControl.h>
#include <UI/Layouts/UILayoutIsolationComponent.h>
#include <UI/UIControlSystem.h>
#include <UI/Layouts/UILayoutSystem.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Base/BaseTypes.h>

using namespace DAVA;

namespace EditorControlsViewDetails
{
class GridControl : public UIControl, DAVA::TArc::DataListener
{
public:
    GridControl(DAVA::TArc::ContextAccessor* accessor);
    ~GridControl() override;
    void SetSize(const Vector2& size) override;

protected:
    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const Vector<Any>& fields) override;

private:
    void Draw(const UIGeometricData& geometricData) override;
    void UpdateColorControlBackground();
    ScopedPtr<UIControl> colorControl;

    DAVA::TArc::ContextAccessor* accessor = nullptr;
    DAVA::TArc::DataWrapper wrapper;
};

GridControl::GridControl(DAVA::TArc::ContextAccessor* accessor_)
    : colorControl(new UIControl)
    , accessor(accessor_)
{
    {
        UIControlBackground* background = GetOrCreateComponent<UIControlBackground>();
        background->SetDrawType(UIControlBackground::DRAW_TILED);
        ScopedPtr<Sprite> sprite(Sprite::CreateFromSourceFile("~res:/QuickEd/UI/GrayGrid.png"));
        background->SetSprite(sprite, 0);
        colorControl->SetName("Color_control");
    }

    {
        UIControlBackground* background = colorControl->GetOrCreateComponent<UIControlBackground>();
        background->SetDrawType(UIControlBackground::DRAW_FILL);
        UIControl::AddControl(colorControl);
    }

    UpdateColorControlBackground();
    wrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<PreviewWidgetSettings>());
    wrapper.SetListener(this);
}

GridControl::~GridControl()
{
    wrapper.SetListener(nullptr);
}

void GridControl::SetSize(const Vector2& size)
{
    colorControl->SetSize(size);
    UIControl::SetSize(size);
}

void GridControl::Draw(const UIGeometricData& geometricData)
{
    if (0.0f != geometricData.scale.x)
    {
        float32 invScale = 1.0f / geometricData.scale.x;
        UIGeometricData unscaledGd;
        unscaledGd.scale = Vector2(invScale, invScale);
        unscaledGd.size = geometricData.size * geometricData.scale.x;
        unscaledGd.AddGeometricData(geometricData);
        UIControl::Draw(unscaledGd);
    }
}

void GridControl::OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const Vector<Any>& fields)
{
    DVASSERT(wrapper == this->wrapper);
    UpdateColorControlBackground();
}

void GridControl::UpdateColorControlBackground()
{
    PreviewWidgetSettings* settings = accessor->GetGlobalContext()->GetData<PreviewWidgetSettings>();
    DAVA::Color color = settings->backgroundColors[settings->backgroundColorIndex];
    colorControl->GetBackground()->SetColor(color);
}
} //EditorControlsViewDetails

class BackgroundController final
{
public:
    BackgroundController(UIControl* nestedControl, DAVA::TArc::ContextAccessor* accessor);
    ~BackgroundController();
    UIControl* GetGridControl() const;
    bool IsNestedControl(const UIControl* control) const;
    Vector2 GetRootControlPos() const;

    void RecalculateBackgroundProperties(DAVA::UIControl* control);
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from);
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* /*destination*/, int /*index*/);
    void UpdateCounterpoise();
    void AdjustToNestedControl();
    static bool IsPropertyAffectBackground(AbstractProperty* property);

    Signal<> contentSizeChanged;
    Signal<> rootControlPosChanged;

private:
    void CalculateTotalRect(Rect& totalRect, Vector2& rootControlPosition) const;
    void FitGridIfParentIsNested(DAVA::UIControl* control);
    RefPtr<UIControl> gridControl;
    RefPtr<UIControl> counterpoiseControl;
    RefPtr<UIControl> positionHolderControl;

    DAVA::Vector2 rootControlPos;
    UIControl* nestedControl = nullptr;
};

BackgroundController::BackgroundController(UIControl* nestedControl_, DAVA::TArc::ContextAccessor* accessor)
    : gridControl(new EditorControlsViewDetails::GridControl(accessor))
    , counterpoiseControl(new UIControl())
    , positionHolderControl(new UIControl())
    , nestedControl(nestedControl_)
{
    DVASSERT(nullptr != nestedControl);
    String name = nestedControl->GetName().c_str();
    name = name.empty() ? "unnamed" : name;
    gridControl->SetName(FastName("Grid_control_of_nestedControl"));
    counterpoiseControl->SetName(FastName("counterpoise_of_nestedControl"));
    positionHolderControl->SetName(FastName("Position_holder_of_nestedControl"));
    gridControl->AddControl(positionHolderControl.Get());
    positionHolderControl->AddControl(counterpoiseControl.Get());
    counterpoiseControl->AddControl(nestedControl);
    nestedControl->GetOrCreateComponent<UILayoutIsolationComponent>();
}

BackgroundController::~BackgroundController()
{
    nestedControl->RemoveComponent<UILayoutIsolationComponent>();
}

UIControl* BackgroundController::GetGridControl() const
{
    return gridControl.Get();
}

bool BackgroundController::IsNestedControl(const UIControl* control) const
{
    return control == nestedControl;
}

DAVA::Vector2 BackgroundController::GetRootControlPos() const
{
    return rootControlPos;
}

void BackgroundController::RecalculateBackgroundProperties(DAVA::UIControl* control)
{
    if (control == nestedControl)
    {
        UpdateCounterpoise();
    }
    FitGridIfParentIsNested(control);
}

namespace
{
void CalculateTotalRectImpl(UIControl* control, Rect& totalRect, Vector2& rootControlPosition, const UIGeometricData& gd)
{
    if (!control->GetVisibilityFlag() || control->IsHiddenForDebug())
    {
        return;
    }
    UIGeometricData tempGeometricData = control->GetLocalGeometricData();
    tempGeometricData.AddGeometricData(gd);
    Rect box = tempGeometricData.GetAABBox();

    if (totalRect.x > box.x)
    {
        float32 delta = totalRect.x - box.x;
        rootControlPosition.x += delta;
        totalRect.dx += delta;
        totalRect.x = box.x;
    }
    if (totalRect.y > box.y)
    {
        float32 delta = totalRect.y - box.y;
        rootControlPosition.y += delta;
        totalRect.dy += delta;
        totalRect.y = box.y;
    }
    if (totalRect.x + totalRect.dx < box.x + box.dx)
    {
        float32 nextRight = box.x + box.dx;
        totalRect.dx = nextRight - totalRect.x;
    }
    if (totalRect.y + totalRect.dy < box.y + box.dy)
    {
        float32 nextBottom = box.y + box.dy;
        totalRect.dy = nextBottom - totalRect.y;
    }

    for (const auto& child : control->GetChildren())
    {
        CalculateTotalRectImpl(child, totalRect, rootControlPosition, tempGeometricData);
    }
}
} //unnamed namespace

void BackgroundController::CalculateTotalRect(Rect& totalRect, Vector2& rootControlPosition) const
{
    rootControlPosition.SetZero();
    UIGeometricData gd = nestedControl->GetGeometricData();

    gd.position.SetZero();
    UIControl* scalableControl = gridControl->GetParent()->GetParent();
    DVASSERT(nullptr != scalableControl, "grid update without being attached to screen");
    gd.scale /= scalableControl->GetScale(); //grid->controlCanvas->scalableControl
    if (gd.scale.x != 0.0f || gd.scale.y != 0.0f)
    {
        totalRect = gd.GetAABBox();

        for (const auto& child : nestedControl->GetChildren())
        {
            CalculateTotalRectImpl(child, totalRect, rootControlPosition, gd);
        }
    }
}

void BackgroundController::AdjustToNestedControl()
{
    Rect rect;
    Vector2 pos;
    CalculateTotalRect(rect, pos);
    Vector2 size = rect.GetSize();
    positionHolderControl->SetPosition(pos);
    gridControl->SetSize(size);
    rootControlPos = pos;
    rootControlPosChanged.Emit();
}

void BackgroundController::ControlWasRemoved(ControlNode* node, ControlsContainerNode* from)
{
    //check that we adjust removed node
    if (node->GetControl() == nestedControl)
    {
        return;
    }
    FitGridIfParentIsNested(from->GetControl());
}

void BackgroundController::ControlWasAdded(ControlNode* /*node*/, ControlsContainerNode* destination, int /*index*/)
{
    FitGridIfParentIsNested(destination->GetControl());
}

void BackgroundController::UpdateCounterpoise()
{
    UIGeometricData gd = nestedControl->GetLocalGeometricData();
    Vector2 invScale;
    invScale.x = gd.scale.x != 0.0f ? 1.0f / gd.scale.x : 0.0f;
    invScale.y = gd.scale.y != 0.0f ? 1.0f / gd.scale.y : 0.0f;
    counterpoiseControl->SetScale(invScale);
    counterpoiseControl->SetSize(gd.size);
    gd.cosA = cosf(gd.angle);
    gd.sinA = sinf(gd.angle);
    counterpoiseControl->SetAngle(-gd.angle);
    Vector2 pos(gd.position * invScale);
    Vector2 angeledPosition(pos.x * gd.cosA + pos.y * gd.sinA,
                            pos.x * -gd.sinA + pos.y * gd.cosA);

    counterpoiseControl->SetPosition(-angeledPosition + gd.pivotPoint);
}

void BackgroundController::FitGridIfParentIsNested(UIControl* control)
{
    UIControl* parent = control;
    while (parent != nullptr)
    {
        if (parent == nestedControl) //we change child in the nested control
        {
            AdjustToNestedControl();
            contentSizeChanged.Emit();
            return;
        }
        parent = parent->GetParent();
    }
}

bool BackgroundController::IsPropertyAffectBackground(AbstractProperty* property)
{
    DVASSERT(nullptr != property);
    FastName name(property->GetName());
    static FastName matchedNames[] = { FastName("angle"), FastName("size"), FastName("scale"), FastName("position"), FastName("pivot"), FastName("visible") };
    return std::find(std::begin(matchedNames), std::end(matchedNames), name) != std::end(matchedNames);
}

EditorControlsView::EditorControlsView(DAVA::UIControl* canvas, DAVA::TArc::ContextAccessor* accessor)
    : BaseEditorSystem(accessor)
    , controlsCanvas(new UIControl())
    , packageListenerProxy(this, accessor)
{
    canvas->AddControl(controlsCanvas.Get());
    controlsCanvas->SetName(FastName("controls_canvas"));

    canvasDataWrapper = accessor->CreateWrapper(ReflectedTypeDB::Get<CanvasData>());

    GetEngineContext()->uiControlSystem->GetLayoutSystem()->AddListener(this);

    Engine::Instance()->beginFrame.Connect(this, &EditorControlsView::PlaceControlsOnCanvas);
    Engine::Instance()->gameLoopStopped.Connect(this, &EditorControlsView::OnGameLoopStopped);
}

EditorControlsView::~EditorControlsView()
{
}

void EditorControlsView::DeleteCanvasControls(const CanvasControls& canvasControls)
{
}

void EditorControlsView::OnDragStateChanged(EditorSystemsManager::eDragState /*currentState*/, EditorSystemsManager::eDragState previousState)
{
    if (previousState == EditorSystemsManager::Transform)
    {
        for (auto& control : gridControls)
        {
            control->UpdateCounterpoise();
            control->AdjustToNestedControl();
        }
    }
    needRecalculateBgrBeforeRender = true;
}

void EditorControlsView::ControlWasRemoved(ControlNode* node, ControlsContainerNode* from)
{
    if (nullptr == controlsCanvas->GetParent())
    {
        return;
    }
    for (auto& iter : gridControls)
    {
        iter->ControlWasRemoved(node, from);
    }
}

void EditorControlsView::ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index)
{
    if (nullptr == controlsCanvas->GetParent())
    {
        return;
    }
    for (auto& iter : gridControls)
    {
        iter->ControlWasAdded(node, destination, index);
    }
}

void EditorControlsView::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    DVASSERT(nullptr != node);
    DVASSERT(nullptr != property);

    if (nullptr == controlsCanvas->GetParent()) //detached canvas
    {
        DVASSERT(false);
        return;
    }

    if (GetSystemsManager()->GetDragState() != EditorSystemsManager::Transform)
    {
        if (BackgroundController::IsPropertyAffectBackground(property))
        {
            RecalculateBackgroundPropertiesForGrids(node->GetControl());
        }
    }
}

void EditorControlsView::OnControlLayouted(UIControl* control)
{
    needRecalculateBgrBeforeRender = true;
}

void EditorControlsView::RecalculateBackgroundPropertiesForGrids(DAVA::UIControl* control)
{
    for (auto& iter : gridControls)
    {
        iter->RecalculateBackgroundProperties(control);
    }
}

BaseEditorSystem::eSystems EditorControlsView::GetOrder() const
{
    return CONTROLS_VIEW;
}

void EditorControlsView::OnUpdate()
{
    if (needRecalculateBgrBeforeRender)
    {
        if (GetSystemsManager()->GetDragState() == EditorSystemsManager::Transform)
        { // do not recalculate while control is dragged
            return;
        }

        needRecalculateBgrBeforeRender = false;

        for (auto& iter : gridControls)
        {
            iter->UpdateCounterpoise();
            iter->AdjustToNestedControl();
        }
        Layout();
    }
}

void EditorControlsView::PlaceControlsOnCanvas()
{
    static SortedControlNodeSet displayedControls;
    SortedControlNodeSet newDisplayedControls = GetDisplayedControls();
    if (displayedControls != newDisplayedControls)
    {
        OnRootContolsChanged(newDisplayedControls, displayedControls);

        //we need to retain cached nodes between frames
        //this is the only way to have no conflicts with the other systems
        for (ControlNode* node : displayedControls)
        {
            node->Release();
        }
        displayedControls = newDisplayedControls;
        for (ControlNode* node : displayedControls)
        {
            node->Retain();
        }
    }
}

BackgroundController* EditorControlsView::CreateControlBackground(PackageBaseNode* node)
{
    BackgroundController* backgroundController(new BackgroundController(node->GetControl(), accessor));
    backgroundController->contentSizeChanged.Connect(this, &EditorControlsView::Layout);
    backgroundController->rootControlPosChanged.Connect(this, &EditorControlsView::OnRootControlPosChanged);
    gridControls.emplace_back(backgroundController);
    return backgroundController;
}

void EditorControlsView::AddBackgroundControllerToCanvas(BackgroundController* backgroundController, size_t pos)
{
    UIControl* grid = backgroundController->GetGridControl();
    if (pos >= controlsCanvas->GetChildren().size())
    {
        controlsCanvas->AddControl(grid);
    }
    else
    {
        auto iterToInsert = controlsCanvas->GetChildren().begin();
        std::advance(iterToInsert, pos);
        controlsCanvas->InsertChildBelow(grid, *iterToInsert);
    }
    backgroundController->UpdateCounterpoise();
    backgroundController->AdjustToNestedControl();
}

uint32 EditorControlsView::GetIndexByPos(const Vector2& pos) const
{
    uint32 index = 0;
    for (auto& iter : gridControls)
    {
        auto grid = iter->GetGridControl();

        if (pos.y < (grid->GetPosition().y + grid->GetSize().y / 2.0f))
        {
            return index;
        }
        index++;
    }
    return index;
}

void EditorControlsView::Layout()
{
    if (canvasDataWrapper.HasData() == false)
    {
        return;
    }

    float32 maxWidth = 0.0f;
    float32 totalHeight = 0.0f;
    const int spacing = 5;
    const List<UIControl*>& children = controlsCanvas->GetChildren();
    size_t childrenCount = children.size();
    if (childrenCount > 1)
    {
        totalHeight += spacing * (childrenCount - 1);
    }

    //collect current geometry
    for (const UIControl* control : children)
    {
        Vector2 childSize = control->GetSize();
        maxWidth = Max(maxWidth, childSize.dx);
        totalHeight += childSize.dy;
    }

    //place all grids in a column
    float32 curY = 0.0f;
    for (UIControl* child : children)
    {
        Rect rect = child->GetRect();
        rect.y = curY;
        rect.x = (maxWidth - rect.dx) / 2.0f;
        child->SetRect(rect);
        curY += rect.dy + spacing;
    }
    Vector2 size(maxWidth, totalHeight);

    canvasDataWrapper.SetFieldValue(CanvasData::workAreaSizePropertyName, size);

    OnRootControlPosChanged();
}

void EditorControlsView::OnRootContolsChanged(const SortedControlNodeSet& newRootControls, const SortedControlNodeSet& oldRootControls)
{
    //set_difference requires sorted Set without custom comparator
    Set<ControlNode*> sortedNewControls(newRootControls.begin(), newRootControls.end());
    Set<ControlNode*> sortedOldControls(oldRootControls.begin(), oldRootControls.end());

    Set<ControlNode*> newNodes;
    Set<ControlNode*> deletedNodes;
    if (!oldRootControls.empty())
    {
        std::set_difference(sortedOldControls.begin(),
                            sortedOldControls.end(),
                            sortedNewControls.begin(),
                            sortedNewControls.end(),
                            std::inserter(deletedNodes, deletedNodes.end()));
    }
    if (!newRootControls.empty())
    {
        std::set_difference(sortedNewControls.begin(),
                            sortedNewControls.end(),
                            sortedOldControls.begin(),
                            sortedOldControls.end(),
                            std::inserter(newNodes, newNodes.end()));
    }

    for (auto iter = deletedNodes.begin(); iter != deletedNodes.end(); ++iter)
    {
        PackageBaseNode* node = *iter;
        UIControl* control = node->GetControl();
        auto findIt = std::find_if(gridControls.begin(), gridControls.end(), [control](std::unique_ptr<BackgroundController>& gridIter) {
            return gridIter->IsNestedControl(control);
        });
        DVASSERT(findIt != gridControls.end());
        controlsCanvas->RemoveControl(findIt->get()->GetGridControl());
        gridControls.erase(findIt);
    }

    for (auto iter = newRootControls.begin(); iter != newRootControls.end(); ++iter)
    {
        ControlNode* node = *iter;
        if (newNodes.find(node) == newNodes.end())
        {
            continue;
        }
        UIControl* control = node->GetControl();
        DVASSERT(std::find_if(gridControls.begin(), gridControls.end(), [control](std::unique_ptr<BackgroundController>& gridIter) {
                     return gridIter->IsNestedControl(control);
                 }) == gridControls.end());
        BackgroundController* backgroundController = CreateControlBackground(node);
        AddBackgroundControllerToCanvas(backgroundController, std::distance(newRootControls.begin(), iter));
    }
    needRecalculateBgrBeforeRender = true;

    //centralize new displayed root controls while we have no ensure visible functions
    if (canvasDataWrapper.HasData())
    {
        canvasDataWrapper.SetFieldValue(CanvasData::needCentralizePropertyName, true);
    }
}

//later background controls must be a part of data and rootControlPos must be simple getter
void EditorControlsView::OnRootControlPosChanged()
{
    if (gridControls.size() == 1)
    {
        const std::unique_ptr<BackgroundController>& grid = gridControls.front();
        canvasDataWrapper.SetFieldValue(CanvasData::rootPositionPropertyName, grid->GetRootControlPos());
    }
    else
    {
        //force show 0, 0 at top left corner if many root controls displayed
        canvasDataWrapper.SetFieldValue(CanvasData::rootPositionPropertyName, Vector2(0.0f, 0.0f));
    }
}

SortedControlNodeSet EditorControlsView::GetDisplayedControls() const
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    DataContext* activeContext = accessor->GetActiveContext();
    if (nullptr == activeContext)
    {
        return SortedControlNodeSet(CompareByLCA);
    }

    DocumentData* documentData = activeContext->GetData<DocumentData>();
    return documentData->GetDisplayedRootControls();
}

void EditorControlsView::OnGameLoopStopped()
{
    GetEngineContext()->uiControlSystem->GetLayoutSystem()->RemoveListener(this);
    Engine::Instance()->beginFrame.Disconnect(this);
}
