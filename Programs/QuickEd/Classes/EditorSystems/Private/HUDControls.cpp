#include "EditorSystems/HUDControls.h"
#include <Modules/UpdateViewsSystemModule/UpdateViewsSystem.h>

#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/VisibleValueProperty.h"
#include "EditorSystems/EditorTransformSystem.h"

#include <UI/UIControlSystem.h>
#include <UI/Layouts/UIAnchorComponent.h>
#include <Render/2D/Sprite.h>
#include <Render/2D/Systems/RenderSystem2D.h>
#include <Preferences/PreferencesRegistrator.h>
#include <Preferences/PreferencesStorage.h>

using namespace DAVA;

namespace HUDControlsDetails
{
RefPtr<UIControl> CreateFrameBorderControl(FrameControl::eBorder border)
{
    RefPtr<UIControl> control(new UIControl(Rect(1.0f, 1.0f, 1.0f, 1.0f)));
    UIAnchorComponent* anchor = control->GetOrCreateComponent<UIAnchorComponent>();
    anchor->SetLeftAnchorEnabled(true);
    anchor->SetRightAnchorEnabled(true);
    anchor->SetTopAnchorEnabled(true);
    anchor->SetBottomAnchorEnabled(true);
    switch (border)
    {
    case FrameControl::TOP:
        anchor->SetBottomAnchorEnabled(false);
        break;
    case FrameControl::BOTTOM:
        anchor->SetTopAnchorEnabled(false);
        break;
    case FrameControl::LEFT:
        anchor->SetRightAnchorEnabled(false);
        break;
    case FrameControl::RIGHT:
        anchor->SetLeftAnchorEnabled(false);
        break;
    default:
        DVASSERT("!impossible value for frame control position");
    }
    return control;
}

const HUDControlsPreferences& GetPreferences()
{
    static HUDControlsPreferences preferences;
    return preferences;
}
}

REGISTER_PREFERENCES_ON_START(HUDControlsPreferences,
                              PREF_ARG("selectionRectColor", Color(0.8f, 0.8f, 0.8f, 0.9f)),
                              PREF_ARG("highlightColor", Color(0.26f, 0.75f, 1.0f, 0.9f)),
                              PREF_ARG("hudRectColor", Color(0.8f, 0.8f, 0.8f, 0.9f)),
                              PREF_ARG("cornerRectPath2", String("~res:/QuickEd/UI/HUDControls/CornerRect.png")),
                              PREF_ARG("borderRectPath2", String("~res:/QuickEd/UI/HUDControls/BorderRect.png")),
                              PREF_ARG("pivotPointPath2", String("~res:/QuickEd/UI/HUDControls/Pivot.png")),
                              PREF_ARG("rotatePath2", String("~res:/QuickEd/UI/HUDControls/Rotate.png")),
                              PREF_ARG("magnetLinePath2", String("~res:/QuickEd/UI/HUDControls/MagnetLine/dotline.png")),
                              PREF_ARG("magnetRectPath2", String("~res:/QuickEd/UI/HUDControls/MagnetLine/MagnetLine.png"))
                              )

HUDControlsPreferences::HUDControlsPreferences()
{
    PreferencesStorage::Instance()->RegisterPreferences(this);
}

#ifdef IMPL_PREFERENCE
#error "IMPL_PREFERENCE is already declared, rename it tho";
#endif //IMPL_PREFERENCE

#define IMPL_PREFERENCE(T, pref) \
    T HUDControlsPreferences::pref; \
    T HUDControlsPreferences::Get##pref() const \
    { \
        return pref; \
    } \
    void HUDControlsPreferences::Set##pref(const T& arg) \
    { \
        pref = arg; \
    }

IMPL_PREFERENCE(Color, selectionRectColor);
IMPL_PREFERENCE(Color, highlightColor);
IMPL_PREFERENCE(Color, hudRectColor);
IMPL_PREFERENCE(String, cornerRectPath2);
IMPL_PREFERENCE(String, borderRectPath2);
IMPL_PREFERENCE(String, pivotPointPath2);
IMPL_PREFERENCE(String, rotatePath2);
IMPL_PREFERENCE(String, magnetLinePath2);
IMPL_PREFERENCE(String, magnetRectPath2);

#undef IMPL_PREFERENCE

ControlContainer::ControlContainer(const HUDAreaInfo::eArea area_)
    : area(area_)
    , drawable(new UIControl())
{
}

ControlContainer::~ControlContainer() = default;

HUDAreaInfo::eArea ControlContainer::GetArea() const
{
    return area;
}

void ControlContainer::SetSystemVisible(bool visible)
{
    systemVisible = visible;
}

bool ControlContainer::GetSystemVisible() const
{
    return systemVisible;
}

void ControlContainer::AddChild(std::unique_ptr<ControlContainer>&& child)
{
    drawable->AddControl(child->drawable.Get());
    children.emplace_back(std::move(child));
}

void ControlContainer::AddToParent(DAVA::UIControl* parent)
{
    parent->AddControl(drawable.Get());
}

void ControlContainer::RemoveFromParent(DAVA::UIControl* parent)
{
    parent->RemoveControl(drawable.Get());
}

void ControlContainer::SetVisible(bool visible)
{
    drawable->SetVisibilityFlag(visible);
}

void ControlContainer::SetName(const DAVA::String& name)
{
    drawable->SetName(name);
}

bool ControlContainer::IsPointInside(const Vector2& point) const
{
    return drawable->IsPointInside(point);
}

bool ControlContainer::IsHiddenForDebug() const
{
    return drawable->IsHiddenForDebug();
}

bool ControlContainer::GetVisibilityFlag() const
{
    return drawable->GetVisibilityFlag();
}

HUDContainer::HUDContainer(const ControlNode* node_)
    : ControlContainer(HUDAreaInfo::NO_AREA)
    , node(node_)
{
    DVASSERT(nullptr != node);
    drawable->SetName(FastName(String("HudContainer of ") + node->GetName().c_str()));
    control = node->GetControl();
    visibleProperty = node->GetRootProperty()->GetVisibleProperty();
    DVASSERT(nullptr != control && nullptr != visibleProperty);

    UpdateViewsSystem* updateSystem = DAVA::GetEngineContext()->uiControlSystem->GetSystem<UpdateViewsSystem>();
    updateSystem->beforeRender.Connect(this, &HUDContainer::OnUpdate);
}

void HUDContainer::InitFromGD(const UIGeometricData& gd)
{
    bool contolIsInValidState = systemVisible && gd.size.dx >= 0.0f && gd.size.dy >= 0.0f && gd.scale.dx > 0.0f && gd.scale.dy > 0.0f;
    bool containerVisible = contolIsInValidState && visibleProperty->GetVisibleInEditor();
    if (containerVisible)
    {
        PackageBaseNode* parent = node->GetParent();
        while (containerVisible && nullptr != parent)
        {
            ControlNode* parentControlNode = dynamic_cast<ControlNode*>(parent);
            if (parentControlNode == nullptr)
            {
                break;
            }
            containerVisible &= parentControlNode->GetRootProperty()->GetVisibleProperty()->GetVisibleInEditor();
            parent = parent->GetParent();
        }
    }
    drawable->SetVisibilityFlag(containerVisible);
    if (containerVisible)
    {
        const DAVA::Vector2& minimumSize = EditorTransformSystem::GetMinimumSize();
        DAVA::Vector2 actualSize(gd.size * gd.scale);
        DAVA::UIGeometricData changedGD = gd;
        bool controlIsMoveOnly = actualSize.dx < minimumSize.dx && actualSize.dy < minimumSize.dy;
        if (controlIsMoveOnly)
        {
            changedGD.position -= ::Rotate((minimumSize - actualSize) / 2.0f, changedGD.angle);
            changedGD.size = minimumSize / gd.scale;
        }

        Rect ur(changedGD.position - ::Rotate(changedGD.pivotPoint, changedGD.angle) * changedGD.scale, changedGD.size * changedGD.scale);
        drawable->SetRect(ur);

        drawable->SetAngle(changedGD.angle);

        for (const std::unique_ptr<ControlContainer>& child : children)
        {
            HUDAreaInfo::eArea area = child->GetArea();
            bool childVisible = child->GetSystemVisible() && changedGD.scale.x > 0.0f && changedGD.scale.y > 0.0f;
            if (area != HUDAreaInfo::FRAME_AREA)
            {
                childVisible &= !controlIsMoveOnly;
            }
            child->SetVisible(childVisible);
            if (childVisible)
            {
                child->InitFromGD(changedGD);
            }
        }
    }
}

void HUDContainer::OnUpdate()
{
    auto controlGD = control->GetGeometricData();
    InitFromGD(controlGD);
}

FrameControl::FrameControl(eType type_)
    : ControlContainer(HUDAreaInfo::FRAME_AREA)
    , type(type_)
{
    drawable->SetName(FastName("Frame Control"));

    switch (type)
    {
    case SELECTION:
    case SELECTION_RECT:
        lineThickness = 1.0f;
        break;
    case HIGHLIGHT:
        lineThickness = 2.0f;
        break;
    default:
        DVASSERT(false, "set line thickness please");
        break;
    }

    for (uint32 i = 0; i < eBorder::COUNT; ++i)
    {
        FrameControl::eBorder border = static_cast<FrameControl::eBorder>(i);

        RefPtr<UIControl> control(new UIControl());
        control->SetName(FastName(String("border of ") + drawable->GetName().c_str()));
        UIControlBackground* background = control->GetOrCreateComponent<UIControlBackground>();
        background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
        background->SetDrawType(UIControlBackground::DRAW_FILL);
        switch (type)
        {
        case SELECTION:
            background->SetColor(HUDControlsDetails::GetPreferences().hudRectColor);
            break;
        case SELECTION_RECT:
            background->SetColor(HUDControlsDetails::GetPreferences().selectionRectColor);
            break;
        case HIGHLIGHT:
            background->SetColor(HUDControlsDetails::GetPreferences().highlightColor);
            break;
        default:
            break;
        }
        drawable->AddControl(control.Get());
    }
}

void FrameControl::InitFromGD(const UIGeometricData& gd)
{
    Rect currentRect(Vector2(0.0f, 0.0f), gd.size * gd.scale);
    SetRect(currentRect);
}

void FrameControl::SetRect(const DAVA::Rect& rect)
{
    drawable->SetRect(rect);
    List<UIControl*> children = drawable->GetChildren();
    auto iter = children.begin();
    for (uint32 i = 0; i < eBorder::COUNT; ++i, ++iter)
    {
        FrameControl::eBorder border = static_cast<FrameControl::eBorder>(i);
        (*iter)->SetRect(GetSubControlRect(rect, border));
    }
}

Rect FrameControl::GetAbsoluteRect() const
{
    return drawable->GetAbsoluteRect();
}

Rect FrameControl::GetSubControlRect(const DAVA::Rect& rect, eBorder border) const
{
    switch (border)
    {
    case TOP:
        return Rect(0.0f, 0.0f, rect.dx, lineThickness);
    case BOTTOM:
        return Rect(0.0f, rect.dy - lineThickness, rect.dx, lineThickness);
    case LEFT:
        return Rect(0.0f, 0.0f, lineThickness, rect.dy);
    case RIGHT:
        return Rect(rect.dx - lineThickness, 0.0f, lineThickness, rect.dy);
    default:
        DVASSERT(!"wrong border passed to frame control");
        return Rect(0.0f, 0.0f, 0.0f, 0.0f);
    }
}

FrameRectControl::FrameRectControl(const HUDAreaInfo::eArea area_)
    : ControlContainer(area_)
{
    drawable->SetName(FastName("Frame Rect Control"));
    UIControlBackground* background = drawable->GetOrCreateComponent<UIControlBackground>();
    FilePath spritePath;
    switch (area)
    {
    case HUDAreaInfo::TOP_LEFT_AREA:
    case HUDAreaInfo::TOP_RIGHT_AREA:
    case HUDAreaInfo::BOTTOM_LEFT_AREA:
    case HUDAreaInfo::BOTTOM_RIGHT_AREA:
        spritePath = HUDControlsDetails::GetPreferences().cornerRectPath2;
        rectSize = Vector2(8.0f, 8.0f);
        break;
    case HUDAreaInfo::TOP_CENTER_AREA:
    case HUDAreaInfo::BOTTOM_CENTER_AREA:
    case HUDAreaInfo::CENTER_LEFT_AREA:
    case HUDAreaInfo::CENTER_RIGHT_AREA:
        spritePath = HUDControlsDetails::GetPreferences().borderRectPath2;
        rectSize = Vector2(8.0f, 8.0f);
        break;
    default:
        DVASSERT(false, "invalid area passed to FrameRectControl");
        break;
    }
    ScopedPtr<Sprite> sprite(Sprite::CreateFromSourceFile(spritePath, true, false));
    background->SetSprite(sprite, 0);
    background->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
    background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
}

void FrameRectControl::InitFromGD(const UIGeometricData& gd)
{
    Rect subRect(Vector2(), rectSize);
    Rect parentRect(Vector2(0.0f, 0.0f), gd.size * gd.scale);
    subRect.SetCenter(GetPos(parentRect));
    drawable->SetRect(subRect);
}

Vector2 FrameRectControl::GetPos(const DAVA::Rect& rect) const
{
    switch (area)
    {
    case HUDAreaInfo::TOP_LEFT_AREA:
        return Vector2(0.0f, 0.0f);
    case HUDAreaInfo::TOP_CENTER_AREA:
        return Vector2(rect.dx / 2.0f, 0.0f);
    case HUDAreaInfo::TOP_RIGHT_AREA:
        return Vector2(rect.dx, 0.0f);
    case HUDAreaInfo::CENTER_LEFT_AREA:
        return Vector2(0, rect.dy / 2.0f);
    case HUDAreaInfo::CENTER_RIGHT_AREA:
        return Vector2(rect.dx, rect.dy / 2.0f);
    case HUDAreaInfo::BOTTOM_LEFT_AREA:
        return Vector2(0, rect.dy);
    case HUDAreaInfo::BOTTOM_CENTER_AREA:
        return Vector2(rect.dx / 2.0f, rect.dy);
    case HUDAreaInfo::BOTTOM_RIGHT_AREA:
        return Vector2(rect.dx, rect.dy);
    default:
        DVASSERT(!"wrong area passed to hud control");
        return Vector2(0.0f, 0.0f);
    }
}

PivotPointControl::PivotPointControl()
    : ControlContainer(HUDAreaInfo::PIVOT_POINT_AREA)
{
    drawable->SetName(FastName("pivot point control"));
    UIControlBackground* background = drawable->GetOrCreateComponent<UIControlBackground>();
    ScopedPtr<Sprite> sprite(Sprite::CreateFromSourceFile(HUDControlsDetails::GetPreferences().pivotPointPath2, true, false));
    background->SetSprite(sprite, 0);
    background->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
    background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
}

void PivotPointControl::InitFromGD(const UIGeometricData& gd)
{
    Rect rect(Vector2(), Vector2(15.0f, 15.0f));
    rect.SetCenter(gd.pivotPoint * gd.scale);
    drawable->SetRect(rect);
}

RotateControl::RotateControl()
    : ControlContainer(HUDAreaInfo::ROTATE_AREA)
{
    drawable->SetName(FastName("rotate control"));
    UIControlBackground* background = drawable->GetOrCreateComponent<UIControlBackground>();
    ScopedPtr<Sprite> sprite(Sprite::CreateFromSourceFile(HUDControlsDetails::GetPreferences().rotatePath2, true, false));
    background->SetSprite(sprite, 0);
    background->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
    background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
}

void RotateControl::InitFromGD(const UIGeometricData& gd)
{
    const DAVA::Vector2 rectSize(15.0f, 15.0f);
    Rect rect(Vector2(0.0f, 0.0f), rectSize);
    Rect controlRect(Vector2(0.0f, 0.0f), gd.size * gd.scale);

    const int margin = 5;
    rect.SetCenter(Vector2(controlRect.dx / 2.0f, controlRect.GetPosition().y - rectSize.y - margin));

    drawable->SetRect(rect);
}

void SetupHUDMagnetLineControl(UIControl* control)
{
    UIControlBackground* background = control->GetOrCreateComponent<UIControlBackground>();
    background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
    ScopedPtr<Sprite> sprite(Sprite::CreateFromSourceFile(HUDControlsDetails::GetPreferences().magnetLinePath2));
    background->SetSprite(sprite, 0);
    background->SetDrawType(UIControlBackground::DRAW_TILED);
    control->SetName("Magnet line");
}

void SetupHUDMagnetRectControl(UIControl* parentControl)
{
    for (int i = 0; i < FrameControl::eBorder::COUNT; ++i)
    {
        FrameControl::eBorder border = static_cast<FrameControl::eBorder>(i);
        RefPtr<UIControl> control(HUDControlsDetails::CreateFrameBorderControl(border));

        UIControlBackground* background = control->GetOrCreateComponent<UIControlBackground>();
        background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
        ScopedPtr<Sprite> sprite(Sprite::CreateFromSourceFile(HUDControlsDetails::GetPreferences().magnetRectPath2));
        background->SetSprite(sprite, 0);
        background->SetDrawType(UIControlBackground::DRAW_TILED);
        control->SetName("magnet rect border");

        control->SetName(FastName(String("border of magnet rect")));
        parentControl->AddControl(control.Get());
    }
}

std::unique_ptr<ControlContainer> CreateHighlightRect(const ControlNode* node)
{
    std::unique_ptr<ControlContainer> container(new HUDContainer(node));
    container->SetName("HUD rect container");
    std::unique_ptr<ControlContainer> frame(new FrameControl(FrameControl::HIGHLIGHT));
    frame->SetName("HUD rect frame control");
    container->AddChild(std::move(frame));

    container->InitFromGD(node->GetControl()->GetGeometricData());
    return container;
}
