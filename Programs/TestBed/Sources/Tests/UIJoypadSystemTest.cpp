#include "UIJoypadSystemTest.h"

#include "Engine/Engine.h"
#include "Input/InputSystem.h"
#include "UI/UIControlBackground.h"
#include "UI/Events/UIEventBindingComponent.h"
#include "UI/Events/UIInputEventComponent.h"
#include "UI/Render/UIDebugRenderComponent.h"
#include "UI/Text/UITextComponent.h"

#include "Utils/StringUtils.h"

#include <iomanip>

UIJoypadSystemTest::UIJoypadSystemTest(TestBed& app)
    : BaseScreen(app, "UIJoypadSystemTest")
{
}

void UIJoypadSystemTest::LoadResources()
{
    BaseScreen::LoadResources();

    DefaultUIPackageBuilder pkgBuilder;
    UIPackageLoader().LoadPackage("~res:/UI/JoypadTest.yaml", &pkgBuilder);
    UIControl* base = pkgBuilder.GetPackage()->GetControl("GlobalBase");

    UIControl* stuff = base->FindByName("StuffBase");

    FastName hoverOn("HoverOn");
    FastName hoverOff("HoverOff");
    FastName touchDown("TouchDown");
    FastName touchUpIn("TouchUpIn");
    FastName touchUpOut("TouchUpOut");

    areaSprites.first = Sprite::CreateFromSourceFile("~res:/TestData/UIJoypadSystemTest/area_active.png", true);
    areaSprites.second = Sprite::CreateFromSourceFile("~res:/TestData/UIJoypadSystemTest/area_inactive.png", true);
    armSprites.first = Sprite::CreateFromSourceFile("~res:/TestData/UIJoypadSystemTest/arm_active.png", true);
    armSprites.second = Sprite::CreateFromSourceFile("~res:/TestData/UIJoypadSystemTest/arm_inactive.png", true);

    for (UIControl* c : stuff->GetChildren())
    {
        if (StringUtils::StartsWith(c->GetName().c_str(), "Btn"))
        {
            UIEventBindingComponent* binding = c->GetOrCreateComponent<UIEventBindingComponent>();

            binding->BindAction(hoverOn, [c]() { c->AddClass(FastName("hoverOn")); });

            binding->BindAction(hoverOff, [c]() { c->RemoveClass(FastName("hoverOn")); });

            binding->BindAction(
            touchDown,
            [c]() {
                c->RemoveClass(FastName("hoverOn"));
                c->AddClass(FastName("touchDown"));
            }
            );

            binding->BindAction(
            touchUpIn,
            [this, c]() {
                c->RemoveClass(FastName("touchDown"));
                c->AddClass(FastName("hoverOn"));

                if (StringUtils::StartsWith(c->GetName().c_str(), "BtnToggleStick"))
                {
                    if (StringUtils::EndsWith(c->GetName().c_str(), "Area"))
                        ToggleStickElement(eStickElements::STICK_AREA, c);
                    if (StringUtils::EndsWith(c->GetName().c_str(), "Arm"))
                        ToggleStickElement(eStickElements::STICK_ARM, c);
                    if (StringUtils::EndsWith(c->GetName().c_str(), "Arrow"))
                        ToggleStickElement(eStickElements::STICK_ARROW, c);
                }
            }
            );

            binding->BindAction(
            touchUpOut,
            [c]() {
                c->RemoveClass(FastName("touchDown"));
                c->RemoveClass(FastName("hoverOn"));
            }
            );
        }
    }

    toggleVisibility = stuff->FindByName("BtnToggleVisibility");
    toggleDynamic = stuff->FindByName("BtnToggleDynamic");
    cancelZone = stuff->FindByName("CancelZone");
    radius = static_cast<UITextField*>(stuff->FindByName("Radius"));
    coords = stuff->FindByName("Coords");

    toggleVisibility->GetComponent<UIEventBindingComponent>()->BindAction(
    touchUpIn,
    [this]() {
        toggleVisibility->RemoveClass(FastName("touchDown"));
        toggleVisibility->AddClass(FastName("hoverOn"));
        ToggleVisibility();
    }
    );

    toggleDynamic->GetComponent<UIEventBindingComponent>()->BindAction(
    touchUpIn,
    [this]() {
        toggleDynamic->RemoveClass(FastName("touchDown"));
        toggleDynamic->AddClass(FastName("hoverOn"));
        ToggleDynamic();
    }
    );

    AddControl(base);

    UIControl* joypadBase = base->FindByName("JoypadBase");

    joypad = joypadBase->GetComponent<UIJoypadComponent>();

    joypad->SetInitialPosition({ 10.f, GetRect().dy - 100.f });
    joypad->SetCoordsTransformFunction(UIJoypadComponent::CoordsTransformFn(
    [](Vector2 v) {
        if (v.Length() < 0.2f)
        {
            return Vector2::Zero;
        }
        return v;
    }
    ));

    UIControl* area = joypadBase->FindByName("StickArea");
    UIControl* arm = joypadBase->FindByName("StickArm");
    UIControl* arrow = joypadBase->FindByName("StickArrow");

    stickElements[eStickElements::STICK_AREA] = area;
    stickElements[eStickElements::STICK_ARM] = arm;
    stickElements[eStickElements::STICK_ARROW] = arrow;

    wasActive = true; // set to true to do initial draw

    inputHandler = GetEngineContext()->inputSystem->AddHandler(eInputDeviceTypes::CLASS_KEYBOARD, MakeFunction(this, &UIJoypadSystemTest::OnInputEvent));

    Engine::Instance()->update.Connect(this, &UIJoypadSystemTest::OnUpdate);
}

void UIJoypadSystemTest::UnloadResources()
{
    Engine::Instance()->update.Disconnect(this);

    GetEngineContext()->inputSystem->RemoveHandler(inputHandler);

    joypad.Set(nullptr);
    toggleVisibility.Set(nullptr);
    toggleDynamic.Set(nullptr);
    cancelZone.Set(nullptr);
    coords.Set(nullptr);
    radius.Set(nullptr);

    BaseScreen::UnloadResources();
}

void UIJoypadSystemTest::OnUpdate(float32 timeElapsed)
{
    if (joypad != nullptr)
    {
        bool isActive = joypad->IsActive();

        if (isActive || wasActive)
        {
            auto& tmp = cancelZone->GetAbsoluteRect();
            joypad->SetCancelZone({ tmp.x, tmp.y, tmp.dx, tmp.dy });

            Vector2 joypadPos = joypad->GetTransformedCoords();

            std::stringstream sst;
            sst << std::fixed << std::setprecision(2) << joypadPos.x << ", " << joypadPos.y;
            coords->GetComponent<UITextComponent>()->SetText(sst.str());

            if (wasActive)
            {
                if (joypad->GetStickArea() != nullptr)
                {
                    UIControlBackground* bg = joypad->GetStickArea()->GetOrCreateComponent<UIControlBackground>();
                    bg->SetSprite((isActive ? areaSprites.first.Get() : areaSprites.second.Get()));
                }

                if (joypad->GetStickArm() != nullptr)
                {
                    UIControlBackground* bg = joypad->GetStickArm()->GetOrCreateComponent<UIControlBackground>();
                    bg->SetSprite((isActive ? armSprites.first.Get() : armSprites.second.Get()));
                }
            }

            coords->GetComponent<UIDebugRenderComponent>()->SetDrawColor((isActive ? Color::Green : Color::Red));
        }

        wasActive = isActive;
    }
}

void UIJoypadSystemTest::ToggleVisibility()
{
    UIControl* base = joypad->GetControl();

    bool isVisibleNow = !base->IsVisible();

    base->SetVisibilityFlag(isVisibleNow);
    toggleVisibility->GetComponent<UIDebugRenderComponent>()->SetDrawColor(isVisibleNow ? Color::Green : Color::Red);
    toggleVisibility->GetComponent<UITextComponent>()->SetText(String("Toggle visibility (KB_V) ") + (isVisibleNow ? "[ON]" : "[OFF]"));
}

void UIJoypadSystemTest::ToggleDynamic()
{
    bool isDynamicNow = !joypad->IsDynamic();

    joypad->SetDynamicFlag(isDynamicNow);
    toggleDynamic->GetComponent<UIDebugRenderComponent>()->SetDrawColor(isDynamicNow ? Color::Green : Color::Red);
    toggleDynamic->GetComponent<UITextComponent>()->SetText(String("Toggle dynamic (KB_V) ") + (isDynamicNow ? "[ON]" : "[OFF]"));
}

void UIJoypadSystemTest::ToggleStickElement(eStickElements e, UIControl* c)
{
    UIControl* element = stickElements[e];

    bool willBeRemoved = false;

    switch (e)
    {
    case UIJoypadSystemTest::STICK_AREA:
        willBeRemoved = joypad->GetStickArea() != nullptr;
        joypad->SetStickAreaControlPath(willBeRemoved ? String() : element->GetName().c_str());
        break;
    case UIJoypadSystemTest::STICK_ARM:
        willBeRemoved = joypad->GetStickArm() != nullptr;
        joypad->SetStickArmControlPath(willBeRemoved ? String() : element->GetName().c_str());
        break;
    case UIJoypadSystemTest::STICK_ARROW:
        willBeRemoved = joypad->GetStickArrow() != nullptr;
        joypad->SetStickArrowControlPath(willBeRemoved ? String() : element->GetName().c_str());
        break;
    default:
        break;
    }

    element->SetVisibilityFlag(!willBeRemoved);

    c->GetComponent<UIDebugRenderComponent>()->SetDrawColor(willBeRemoved ? Color::Red : Color::Green);

    UITextComponent* t = c->GetComponent<UITextComponent>();
    String s = t->GetText();
    t->SetText(s.substr(0, s.find("[")) + String(willBeRemoved ? "[OFF]" : "[ON]"));
}

void UIJoypadSystemTest::SetRadius()
{
    radius->ReleaseFocus();

    float32 r;

    try
    {
        r = std::stof(radius->GetUtf8Text());
    }
    catch (const std::exception&)
    {
        DVASSERT(false, "Please, provide more reasonable value [20 : 1e9]");
        return;
    }

    if (20.f <= r && r <= 1e9f)
    {
        joypad->SetCancelRadius(r);
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

    UIControl* base = joypad->GetControl()->GetParent();
    bool handled = true;

    switch (event.elementId)
    {
    case eInputElements::KB_V:
        ToggleVisibility();
        break;
    case eInputElements::KB_D:
        ToggleDynamic();
        break;
    case eInputElements::KB_A:
        ToggleStickElement(eStickElements::STICK_AREA, base->FindByName("BtnToggleStickArea"));
        break;
    case eInputElements::KB_M:
        ToggleStickElement(eStickElements::STICK_ARM, base->FindByName("BtnToggleStickArm"));
        break;
    case eInputElements::KB_W:
        ToggleStickElement(eStickElements::STICK_ARROW, base->FindByName("BtnToggleStickArrow"));
        break;
    case eInputElements::KB_ENTER:
        SetRadius();
        break;
    default:
        handled = false;
        break;
    }

    return handled;
}
