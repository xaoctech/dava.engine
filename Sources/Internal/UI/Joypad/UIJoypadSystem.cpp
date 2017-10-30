#include "UI/Joypad/UIJoypadSystem.h"
#include "UI/Joypad/UIJoypadComponent.h"
#include "UI/UIControl.h"
#include "UI/UIControlSystem.h"

namespace DAVA
{
void UIJoypadSystem::RegisterControl(UIControl* control)
{
    if (control != nullptr && control->GetComponent(Type::Instance<UIJoypadComponent>()) != nullptr)
    {
        AddComponent(control->GetComponent<UIJoypadComponent>());
    }
}

void UIJoypadSystem::UnregisterControl(UIControl* control)
{
    if (control != nullptr && control->GetComponent(Type::Instance<UIJoypadComponent>()) != nullptr)
    {
        RemoveComponent(control->GetComponent<UIJoypadComponent>());
    }
}

void UIJoypadSystem::RegisterComponent(UIControl* control, UIComponent* component)
{
    if (component != nullptr && component->GetType() == Type::Instance<UIJoypadComponent>())
    {
        AddComponent(static_cast<UIJoypadComponent*>(component));
    }
}

void UIJoypadSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    if (component != nullptr && component->GetType() == Type::Instance<UIJoypadComponent>())
    {
        RemoveComponent(static_cast<UIJoypadComponent*>(component));
    }
}

void UIJoypadSystem::OnControlVisible(UIControl* control)
{
    if (control == nullptr || control->GetComponent(Type::Instance<UIJoypadComponent>()) == nullptr)
    {
        return;
    }

    UIJoypadComponent* component = control->GetComponent<UIJoypadComponent>();

    UIControl* area = component->GetStickArea();
    UIControl* arm = component->GetStickArm();

    if (area != nullptr)
    {
        area->SetVisibilityFlag(true);
    }

    if (arm != nullptr)
    {
        arm->SetVisibilityFlag(true);
    }

    // Component state should be fine, no need to reset it
}

void UIJoypadSystem::OnControlInvisible(UIControl* control)
{
    if (control == nullptr || control->GetComponent(Type::Instance<UIJoypadComponent>()) == nullptr)
    {
        return;
    }

    UIJoypadComponent* component = control->GetComponent<UIJoypadComponent>();

    for (UIControl* c : { component->GetStickArea(), component->GetStickArm(), component->GetStickArrow() })
    {
        if (c != nullptr)
        {
            c->SetVisibilityFlag(false);
        }
    }

    componentsStates[component] = {};
}

void UIJoypadSystem::Process(float32 elapsedTime)
{
    UIControlSystem* scene = GetScene();

    if (scene == nullptr)
    {
        return;
    }

    auto ResetState = [this](UIJoypadComponent* component) {
        DVASSERT(component->GetControl()->GetAbsoluteRect().PointInside(component->GetInitialPosition()));

        ComponentState &state = componentsStates[component];
        state.initialTouch = state.currentTouch = component->GetInitialPosition();
        state.isCanceled = false;

        component->SetActiveFlag(false);

        needUpdate.insert(component);
    };

    for (auto& p : componentsStates)
    {
        UIJoypadComponent* component = p.first;
        ComponentState &state = p.second;

        DVASSERT(component != nullptr);

        UIControl* control = component->GetControl();

        if (control == nullptr)
        {
            continue;
        }

        bool isActive = control->GetState() & UIControl::STATE_PRESSED_INSIDE || control->GetState() & UIControl::STATE_PRESSED_OUTSIDE;

        if (!isActive)
        {
            if (component->IsActive() || state.initialTouch != component->GetInitialPosition())
            {
                ResetState(component);
            }

            continue;
        }

        const Vector<UIEvent>& events = scene->GetAllInputs();
        auto iter = std::find_if(events.begin(), events.end(), [&control](const UIEvent& event) { return event.touchLocker == control; });

        if (iter == events.end())
        {
            DVASSERT(false);
            ResetState(component);
            continue;
        }

        Vector2 touchPoint = iter->point;

        if (!component->IsActive())
        {
            state.initialTouch = (component->IsDynamic() ? touchPoint : component->GetInitialPosition());
        }

        float32 radius = (state.initialTouch - touchPoint).Length();
        bool isCanceled = component->GetCancelZone().PointInside(touchPoint) || radius > component->GetCancelRadius();

        if (isCanceled)
        {
            if (component->IsActive())
            {
                ResetState(component);
            }

            state.isCanceled = true;

            continue;
        }

        if (!state.isCanceled)
        {
            component->SetActiveFlag(true);

            if (state.currentTouch != touchPoint)
            {
                state.currentTouch = touchPoint;
                needUpdate.insert(component);
            }
        }
    }

    // Update components
    UpdateComponents();

    // Clear set
    needUpdate.clear();
}

void UIJoypadSystem::AddComponent(UIJoypadComponent* ptr)
{
    if (ptr != nullptr)
    {
        componentsStates[ptr] = {};
    }
}

void UIJoypadSystem::RemoveComponent(UIJoypadComponent* ptr)
{
    componentsStates.erase(ptr);
}

void UIJoypadSystem::UpdateComponents()
{
    if (needUpdate.empty())
    {
        return;
    }

    auto GetCorrectedPosition = [](UIControl* control, Rect rect) {
        Rect bounds = control->GetAbsoluteRect();

        bounds.x += rect.dx / 2.f;
        bounds.y += rect.dy / 2.f;

        bounds.dx -= rect.dx;
        bounds.dy -= rect.dy;

        Vector2 position = rect.GetPosition();

        bounds.ClampToRect(position);

        return position;
    };

    for (UIJoypadComponent* component : needUpdate)
    {
        if (component == nullptr)
        {
            continue;
        }

        UIControl* control = component->GetControl();

        if (control == nullptr)
        {
            continue;
        }

        const ComponentState& state = componentsStates[component];

        UIControl* stickArea = component->GetStickArea();
        UIControl* stickArm = component->GetStickArm();
        UIControl* stickArrow = component->GetStickArrow();

        float32 radius = component->GetStickAreaRadius();

        Vector2 size = (stickArea != nullptr ? stickArea->GetSize() : Vector2(radius * 2.f, radius * 2.f));

        Vector2 correctedPosition = GetCorrectedPosition(control, { state.initialTouch, size });

        for (UIControl* c : { stickArea, stickArm, stickArrow })
        {
            if (c != nullptr)
            {
                c->SetAbsolutePosition(correctedPosition);
            }
        }

        if (!component->IsActive())
        {
            if (stickArrow != nullptr)
            {
                stickArrow->SetVisibilityFlag(false);
            }

            component->SetOriginalCoords(Vector2::Zero);

            continue;
        }

        Vector2 pos = state.currentTouch - correctedPosition;

        if (stickArm != nullptr)
        {
            DVASSERT(stickArm->GetSize().dx == stickArm->GetSize().dy);

            radius -= stickArm->GetSize().dx / 2.f;
        }

        DVASSERT(radius > 0.f);

        float32 tmp = pos.Length();

        // If stick arm will overlap stick area radius
        if (tmp > radius)
        {
            // Shrink
            pos /= tmp / radius;
        }

        // [-1:1] coords
        Vector2 coords = { pos.x / radius, -pos.y / radius };

        // Compute angle. 0:-PI for the left half, 0:PI for the right
        float32 sm = coords.y;
        float32 mm = coords.Length(); // + SquareRootFloat(1.f * 1.f + 0.f * 0.f)
        float32 cosA = (mm != 0.f ? sm / mm : 0.f);

        if (std::fabs(cosA) > 1.f)
        {
            cosA = (cosA > 0.f ? 1.f : -1.f);
        }

        float32 radA = std::acosf(cosA);

        if (coords.x < 0.f)
        {
            radA = -radA;
        }

        // Set original coords
        component->SetOriginalCoords(coords);

        bool isInsideDeadzone = component->GetTransformedCoords().Length() < component->GetActivationThreshold();

        Vector2 absArmPos = (isInsideDeadzone ? correctedPosition : correctedPosition + pos);

        // Set other stuff
        if (stickArrow != nullptr)
        {
            stickArrow->SetAngle(radA);
            stickArrow->SetVisibilityFlag(!isInsideDeadzone);
        }

        if (stickArm != nullptr)
        {
            stickArm->SetAbsolutePosition(absArmPos);
        }
    }
}
}