#include "UI/Scene3D/UIEntityMarkerSystem.h"
#include "Debug/DVAssert.h"
#include "Scene3D/Components/ScreenPositionComponent.h"
#include "Scene3D/Entity.h"
#include "UI/Scene3D/UIEntityMarkerComponent.h"
#include "UI/UIControl.h"

namespace DAVA
{
UIEntityMarkerSystem::UIEntityMarkerSystem() = default;

UIEntityMarkerSystem::~UIEntityMarkerSystem() = default;

void UIEntityMarkerSystem::RegisterControl(UIControl* control)
{
    UIEntityMarkerComponent* c = control->GetComponent<UIEntityMarkerComponent>();
    if (c)
    {
        components.push_back(c);
    }
}

void UIEntityMarkerSystem::UnregisterControl(UIControl* control)
{
    UIEntityMarkerComponent* c = control->GetComponent<UIEntityMarkerComponent>();
    if (c)
    {
        components.erase(std::remove(components.begin(), components.end(), c), components.end());
    }
}

void UIEntityMarkerSystem::RegisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == Type::Instance<UIEntityMarkerComponent>())
    {
        components.push_back(static_cast<UIEntityMarkerComponent*>(component));
    }
}

void UIEntityMarkerSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == Type::Instance<UIEntityMarkerComponent>())
    {
        components.erase(std::remove(components.begin(), components.end(), static_cast<UIEntityMarkerComponent*>(component)), components.end());
    }
}

void UIEntityMarkerSystem::Process(float32 elapsedTime)
{
    bool hasOrdering = false;
    struct SafeFloat
    {
        float32 val = 0.f;
    };
    Map<UIControl*, Map<UIControl*, SafeFloat>> orderMap;

    for (UIEntityMarkerComponent* component : components)
    {
        if (!component->IsEnabled() || component->GetTargetEntity() == nullptr)
        {
            continue;
        }

        ScreenPositionComponent* spc = static_cast<ScreenPositionComponent*>(component->GetTargetEntity()->GetComponent(Component::SCREEN_POSITION_COMPONENT));
        if (spc)
        {
            Vector3 positionAndDepth = spc->GetScreenPositionAndDepth();
            Vector3 distance = spc->GetWorldPosition() - spc->GetCameraPosition();
            UIControl* control = component->GetControl();

            if (component->IsSyncVisibilityEnabled())
            {
                const bool visible = distance.DotProduct(spc->GetCameraDirection()) > 0.f;
                control->SetVisibilityFlag(visible);

                if (!visible)
                {
                    // Skip next steps for invisible controls
                    continue;
                }
            }

            if (component->IsSyncPositionEnabled())
            {
                control->SetPosition(Vector2(positionAndDepth.x, positionAndDepth.y));
            }

            if (component->IsSyncScaleEnabled())
            {
                if (distance.SquareLength() > 0.f)
                {
                    const Vector2& factor = component->GetScaleFactor();
                    const Vector2& maxScale = component->GetMaxScale();
                    const Vector2& minScale = component->GetMinScale();
                    const float32 distsance = distance.Length();

                    Vector2 scale(Clamp(factor.x / distsance, minScale.x, maxScale.x),
                                  Clamp(factor.y / distsance, minScale.y, maxScale.y));

                    control->SetScale(scale);
                }
                else
                {
                    control->SetScale(component->GetMaxScale());
                }
            }

            if (component->IsSyncOrderEnabled() && control->GetParent())
            {
                hasOrdering = true;
                switch (component->GetOrderMode())
                {
                case UIEntityMarkerComponent::OrderMode::NearFront:
                    orderMap[control->GetParent()][control] = SafeFloat{ -distance.SquareLength() };
                    break;
                case UIEntityMarkerComponent::OrderMode::NearBack:
                    orderMap[control->GetParent()][control] = SafeFloat{ distance.SquareLength() };
                    break;
                }
            }

            if (component->IsUseCustomStrategy() && component->GetCustomStrategy())
            {
                component->GetCustomStrategy()(control, component, spc);
            }
        }
    }

    if (hasOrdering)
    {
        for (auto& pair : orderMap)
        {
            UIControl* parent = pair.first;
            Map<UIControl*, SafeFloat>& order = pair.second;

            parent->SortChildren([&order](UIControl* a, UIControl* b) {
                return order[a].val < order[b].val;
            });
        }
    }
}
}
