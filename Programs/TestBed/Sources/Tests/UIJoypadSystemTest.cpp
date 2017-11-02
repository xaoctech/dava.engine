#include "UIJoypadSystemTest.h"
#include "UI/Joypad/UIJoypadComponent.h"
#include "UI/UIControlBackground.h"
#include "UI/UIControl.h"
#include "UI/Focus/UIFocusComponent.h"
#include "UI/Render/UIDebugRenderComponent.h"
#include "Engine/Engine.h"
#include "Input/InputSystem.h"

#include <iomanip>

struct Joypad
{
    RefPtr<UIControl> base;

    RefPtr<UIControl> stickArea;
    RefPtr<UIControl> stickArm;
    RefPtr<UIControl> stickArrow;

    UIJoypadComponent* component = nullptr;

    RefPtr<Sprite> activeArea;
    RefPtr<Sprite> activeArm;

    RefPtr<Sprite> inactiveArea;
    RefPtr<Sprite> inactiveArm;

    RefPtr<Sprite> arrow;
};

UIJoypadSystemTest::UIJoypadSystemTest(TestBed& app)
    : BaseScreen(app, "UIJoypadSystemTest")
{
}

void UIJoypadSystemTest::LoadResources()
{
    BaseScreen::LoadResources();

    InitControls();

    InitJoypad();

    wasActive = true; // set to true to do initial draw

    inputHandler = GetEngineContext()->inputSystem->AddHandler(eInputDeviceTypes::CLASS_KEYBOARD, MakeFunction(this, &UIJoypadSystemTest::OnInputEvent));

    Engine::Instance()->update.Connect(this, &UIJoypadSystemTest::OnUpdate);
}

void UIJoypadSystemTest::UnloadResources()
{
    Engine::Instance()->update.Disconnect(this);

    GetEngineContext()->inputSystem->RemoveHandler(inputHandler);

    BaseScreen::UnloadResources();

    delete joypad;
}

void UIJoypadSystemTest::OnUpdate(float32 timeElapsed)
{
    if (joypad != nullptr)
    {
        bool isActive = joypad->component->IsActive();

        if (isActive || wasActive)
        {
            Vector2 joypadPos = joypad->component->GetTransformedCoords();

            std::wstringstream wst;
            wst << std::fixed << std::setprecision(2) << joypadPos.x << ", " << joypadPos.y;
            coords->SetStateText(0xFF, wst.str());

            if (wasActive)
            {
                UIJoypadComponent* c = joypad->component;

                if (c->GetStickArea() != nullptr)
                {
                    UIControlBackground* bg = c->GetStickArea()->GetOrCreateComponent<UIControlBackground>();
                    bg->SetSprite((isActive ? joypad->activeArea.Get() : joypad->inactiveArea.Get()));
                }

                if (c->GetStickArm() != nullptr)
                {
                    UIControlBackground* bg = c->GetStickArm()->GetOrCreateComponent<UIControlBackground>();
                    bg->SetSprite((isActive ? joypad->activeArm.Get() : joypad->inactiveArm.Get()));
                }
            }

            coords->GetOrCreateComponent<UIDebugRenderComponent>()->SetDrawColor((isActive ? Color::Green : Color::Red));
        }

        wasActive = isActive;
    }
}

void UIJoypadSystemTest::InitControls()
{
    Font* font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font != nullptr);
    font->SetSize(14.0f);

    coordsText = new UIStaticText({ 30.f, 20.f, 100.f, 30.f });
    coordsText->SetFont(font);
    coordsText->SetText(L"Coords: ");
    AddControl(coordsText.Get());

    coords = new UIButton({ 30.f, 50.f, 100.f, 30.f });
    coords->SetStateFont(0xFF, font);
    coords->SetStateText(0xFF, L"0.00, 0.00");
    coords->GetOrCreateComponent<UIDebugRenderComponent>();
    AddControl(coords.Get());

    toggleVisible = new UIButton({ 190.f, 10.f, 200.f, 30.f });
    toggleVisible->SetStateFont(0xFF, font);
    toggleVisible->SetStateText(0xFF, L"Toggle visibility (on) [KB_V]");
    toggleVisible->GetOrCreateComponent<UIDebugRenderComponent>()->SetDrawColor(Color::Green);
    toggleVisible->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIJoypadSystemTest::ToggleVisible));
    AddControl(toggleVisible.Get());

    toggleDynamic = new UIButton({ 190.f, 50.f, 200.f, 30.f });
    toggleDynamic->SetStateFont(0xFF, font);
    toggleDynamic->SetStateText(0xFF, L"Toggle dynamic (on) [KB_D]");
    toggleDynamic->GetOrCreateComponent<UIDebugRenderComponent>()->SetDrawColor(Color::Green);
    toggleDynamic->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIJoypadSystemTest::ToggleDynamic));
    AddControl(toggleDynamic.Get());

    removeArea = new UIButton({ 190.f, 90.f, 200.f, 30.f });
    removeArea->SetStateFont(0xFF, font);
    removeArea->SetStateText(0xFF, L"Remove area [KB_A]");
    removeArea->GetOrCreateComponent<UIDebugRenderComponent>()->SetDrawColor(Color::Green);
    removeArea->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIJoypadSystemTest::RemoveArea));
    AddControl(removeArea.Get());

    removeArm = new UIButton({ 190.f, 130.f, 200.f, 30.f });
    removeArm->SetStateFont(0xFF, font);
    removeArm->SetStateText(0xFF, L"Remove arm [KB_M]");
    removeArm->GetOrCreateComponent<UIDebugRenderComponent>()->SetDrawColor(Color::Green);
    removeArm->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIJoypadSystemTest::RemoveArm));
    AddControl(removeArm.Get());

    removeArrow = new UIButton({ 190.f, 170.f, 200.f, 30.f });
    removeArrow->SetStateFont(0xFF, font);
    removeArrow->SetStateText(0xFF, L"Remove arrow [KB_W]");
    removeArrow->GetOrCreateComponent<UIDebugRenderComponent>()->SetDrawColor(Color::Green);
    removeArrow->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIJoypadSystemTest::RemoveArrow));
    AddControl(removeArrow.Get());

    cancelZoneText = new UIStaticText({ 400.f, 370.f, 100.f, 30.f });
    cancelZoneText->SetFont(font);
    cancelZoneText->SetText(L"Cancel zone:");
    AddControl(cancelZoneText.Get());

    cancelZone = new UIControl({ 400.f, 400.f, 100.f, 100.f });
    cancelZone->GetOrCreateComponent<UIDebugRenderComponent>();
    AddControl(cancelZone.Get());

    cancelRadiusText = new UIStaticText({ 30.f, 200.f, 110.f, 30.f });
    cancelRadiusText->SetFont(font);
    cancelRadiusText->SetText(L"Cancel radius:");
    AddControl(cancelRadiusText.Get());

    radiusField = new UITextField({ 30.f, 230.f, 110.f, 30.f });
    radiusField->SetFont(font);
    radiusField->SetText(L"Enter radius...");
    radiusField->SetInputEnabled(true);
    radiusField->GetOrCreateComponent<UIFocusComponent>();
    radiusField->GetOrCreateComponent<UIDebugRenderComponent>();
    AddControl(radiusField.Get());

    setRadius = new UIButton({ 190.f, 230.f, 200.f, 30.f });
    setRadius->SetStateFont(0xFF, font);
    setRadius->SetStateText(0xFF, L"Set cancel radius [KB_R]");
    setRadius->GetOrCreateComponent<UIDebugRenderComponent>();
    setRadius->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UIJoypadSystemTest::SetRadius));
    AddControl(setRadius.Get());

    SafeRelease(font);
}

void UIJoypadSystemTest::InitJoypad()
{
    joypad = new Joypad();

    float32 areaSize = 150.f;
    float32 scale = 1.f;

    joypad = new Joypad();

    joypad->base = new UIControl({ 10.f, GetRect().dy - 250.f - 10.f, 300.f, 250.f });
    joypad->base->GetOrCreateComponent<UIDebugRenderComponent>();
    joypad->activeArea = Sprite::CreateFromSourceFile("~res:/TestData/UIJoypadSystemTest/area_active.png");
    joypad->inactiveArea = Sprite::CreateFromSourceFile("~res:/TestData/UIJoypadSystemTest/area_inactive.png");
    joypad->activeArm = Sprite::CreateFromSourceFile("~res:/TestData/UIJoypadSystemTest/arm_active.png");
    joypad->inactiveArm = Sprite::CreateFromSourceFile("~res:/TestData/UIJoypadSystemTest/arm_inactive.png");
    joypad->arrow = Sprite::CreateFromSourceFile("~res:/TestData/UIJoypadSystemTest/arrow.png");

    scale = areaSize / joypad->activeArea->GetSize().dx;

    joypad->stickArea = new UIControl({ { 0.f, 0.f }, joypad->activeArea->GetSize() * scale });
    joypad->stickArm = new UIControl({ { 0.f, 0.f }, joypad->activeArm->GetSize() * scale });
    joypad->stickArrow = new UIControl({ { 0.f, 0.f }, joypad->arrow->GetSize() * scale });
    joypad->stickArrow->SetVisibilityFlag(false);

    joypad->base->AddControl(joypad->stickArea.Get());
    joypad->base->AddControl(joypad->stickArm.Get());
    joypad->base->AddControl(joypad->stickArrow.Get());

    for (UIControl* c : { joypad->stickArea.Get(), joypad->stickArm.Get(), joypad->stickArrow.Get() })
    {
        if (c != nullptr)
        {
            c->GetOrCreateComponent<UIControlBackground>()->SetDrawType(UIControlBackground::eDrawType::DRAW_STRETCH_BOTH);
            c->GetOrCreateComponent<UIControlBackground>()->SetPerPixelAccuracyType(UIControlBackground::ePerPixelAccuracyType::PER_PIXEL_ACCURACY_FORCED);
        }
    }

    joypad->stickArrow->GetComponent<DAVA::UIControlBackground>()->SetSprite(joypad->arrow.Get(), 0);

    joypad->component = joypad->base->GetOrCreateComponent<DAVA::UIJoypadComponent>();
    joypad->component->SetDynamicFlag(true);
    joypad->component->SetStickArea(joypad->stickArea.Get());
    joypad->component->SetStickArm(joypad->stickArm.Get());
    joypad->component->SetStickArrow(joypad->stickArrow.Get());
    joypad->component->SetStickAreaRadius(155.f * scale);
    joypad->component->SetInitialPosition({ 10.f, GetRect().dy - 100.f });
    joypad->component->SetCancelZone(cancelZone->GetAbsoluteRect());
    joypad->component->SetCoordsTransformFunction(UIJoypadComponent::CoordsTransformFn(
    [](Vector2 v) {
        if (v.Length() < 0.2f)
        {
            return Vector2::Zero;
        }
        return v;
    }
    ));

    AddControl(joypad->base.Get());
}

void UIJoypadSystemTest::ToggleVisible(BaseObject*, void*, void*)
{
    if (joypad != nullptr && joypad->base != nullptr)
    {
        if (joypad->base->IsVisible())
        {
            joypad->base->SetVisibilityFlag(false);
            toggleVisible->SetStateText(0xFF, L"Toggle visibility (off) [KB_V]");
            toggleVisible->GetOrCreateComponent<UIDebugRenderComponent>()->SetDrawColor(Color::Red);
        }
        else
        {
            joypad->base->SetVisibilityFlag(true);
            toggleVisible->SetStateText(0xFF, L"Toggle visibility (on) [KB_V]");
            toggleVisible->GetOrCreateComponent<UIDebugRenderComponent>()->SetDrawColor(Color::Green);
        }
    }
}

void UIJoypadSystemTest::ToggleDynamic(BaseObject*, void*, void*)
{
    if (joypad != nullptr && joypad->component != nullptr)
    {
        bool isDynamic = joypad->component->IsDynamic();

        if (isDynamic)
        {
            joypad->component->SetDynamicFlag(false);
            toggleDynamic->SetStateText(0xFF, L"Toggle dynamic (off) [KB_D]");
            toggleDynamic->GetOrCreateComponent<UIDebugRenderComponent>()->SetDrawColor(Color::Red);
        }
        else
        {
            joypad->component->SetDynamicFlag(true);
            toggleDynamic->SetStateText(0xFF, L"Toggle dynamic (on) [KB_D]");
            toggleDynamic->GetOrCreateComponent<UIDebugRenderComponent>()->SetDrawColor(Color::Green);
        }
    }
}

void UIJoypadSystemTest::RemoveArea(BaseObject*, void*, void*)
{
    if (joypad != nullptr && joypad->component != nullptr)
    {
        UIControl* area = joypad->component->GetStickArea();

        if (area != nullptr)
        {
            joypad->component->SetStickArea(nullptr);
            joypad->base->RemoveControl(area);
            removeArea->SetStateText(0xFF, L"Area removed");
            removeArea->GetOrCreateComponent<UIDebugRenderComponent>()->SetDrawColor(Color::Red);
        }
    }
}

void UIJoypadSystemTest::RemoveArm(BaseObject*, void*, void*)
{
    if (joypad != nullptr && joypad->component != nullptr)
    {
        UIControl* arm = joypad->component->GetStickArm();

        if (arm != nullptr)
        {
            joypad->component->SetStickArm(nullptr);
            joypad->base->RemoveControl(arm);
            removeArm->SetStateText(0xFF, L"Arm removed");
            removeArm->GetOrCreateComponent<UIDebugRenderComponent>()->SetDrawColor(Color::Red);
        }
    }
}

void UIJoypadSystemTest::RemoveArrow(BaseObject*, void*, void*)
{
    if (joypad != nullptr && joypad->component != nullptr)
    {
        UIControl* arrow = joypad->component->GetStickArrow();

        if (arrow != nullptr)
        {
            joypad->component->SetStickArrow(nullptr);
            joypad->base->RemoveControl(arrow);
            removeArrow->SetStateText(0xFF, L"Arrow removed");
            removeArrow->GetOrCreateComponent<UIDebugRenderComponent>()->SetDrawColor(Color::Red);
        }
    }
}

void UIJoypadSystemTest::SetRadius(BaseObject*, void*, void*)
{
    float32 radius;

    try
    {
        radius = std::stof(radiusField->GetUtf8Text());
    }
    catch (const std::exception&)
    {
        DVASSERT(false, "Please, provide more reasonable value [20 : 1e9]");
        return;
    }

    if (20.f <= radius && radius <= 1e9f)
    {
        if (joypad != nullptr && joypad->component != nullptr)
        {
            joypad->component->SetCancelRadius(radius);
        }
    }
    else
    {
        DVASSERT(false, "Please, provide more reasonable value [20 : 1e9]");
    }
}

bool UIJoypadSystemTest::OnInputEvent(const InputEvent& event)
{
    // Handle release only
    if (!event.digitalState.IsJustReleased())
    {
        return false;
    }

    bool handled = true;

    switch (event.elementId)
    {
    case eInputElements::KB_V:
        ToggleVisible(nullptr, nullptr, nullptr);
        break;
    case eInputElements::KB_D:
        ToggleDynamic(nullptr, nullptr, nullptr);
        break;
    case eInputElements::KB_A:
        RemoveArea(nullptr, nullptr, nullptr);
        break;
    case eInputElements::KB_M:
        RemoveArm(nullptr, nullptr, nullptr);
        break;
    case eInputElements::KB_W:
        RemoveArrow(nullptr, nullptr, nullptr);
        break;
    case eInputElements::KB_R:
        SetRadius(nullptr, nullptr, nullptr);
        break;
    default:
        handled = false;
        break;
    }

    return handled;
}
