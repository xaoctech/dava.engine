#include "UI/Scene3D/UIEntityMarkerSystem.h"
#include "Debug/DVAssert.h"
#include "Scene3D/Components/ScreenPositionComponent.h"
#include "Scene3D/Entity.h"
#include "UI/Scene3D/UIEntityMarkerComponent.h"
#include "UI/Scene3D/UIEntityMarkerPositionComponent.h"
#include "UI/Scene3D/UIEntityMarkerScaleComponent.h"
#include "UI/Scene3D/UIEntityMarkerVisibilityComponent.h"
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
        Link l;
        l.linkComponent = c;
        l.control = control;
        l.positionComponent = control->GetComponent<UIEntityMarkerPositionComponent>();
        l.scaleComponent = control->GetComponent<UIEntityMarkerScaleComponent>();
        links.push_back(l);
    }
}

void UIEntityMarkerSystem::UnregisterControl(UIControl* control)
{
    links.erase(std::remove_if(links.begin(), links.end(), [control](const Link& l) {
                    return l.control == control;
                }),
                links.end());
}

void UIEntityMarkerSystem::RegisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == Type::Instance<UIEntityMarkerComponent>())
    {
        RegisterControl(control);
    }
    else if (component->GetType() == Type::Instance<UIEntityMarkerPositionComponent>())
    {
        Link* l = Find(control);
        if (l)
        {
            l->positionComponent = static_cast<UIEntityMarkerPositionComponent*>(component);
        }
    }
    else if (component->GetType() == Type::Instance<UIEntityMarkerScaleComponent>())
    {
        Link* l = Find(control);
        if (l)
        {
            l->scaleComponent = static_cast<UIEntityMarkerScaleComponent*>(component);
        }
    }
}

void UIEntityMarkerSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == Type::Instance<UIEntityMarkerComponent>())
    {
        UnregisterControl(control);
    }
    else if (component->GetType() == Type::Instance<UIEntityMarkerPositionComponent>())
    {
        Link* l = Find(control);
        if (l)
        {
            DVASSERT(l->positionComponent == static_cast<UIEntityMarkerPositionComponent*>(component));
            l->positionComponent = nullptr;
        }
    }
    else if (component->GetType() == Type::Instance<UIEntityMarkerScaleComponent>())
    {
        Link* l = Find(control);
        if (l)
        {
            DVASSERT(l->scaleComponent == static_cast<UIEntityMarkerScaleComponent*>(component));
            l->scaleComponent = nullptr;
        }
    }
}

void UIEntityMarkerSystem::Process(float32 elapsedTime)
{
    for (Link& l : links)
    {
        if (!l.linkComponent->GetTargetEntity())
        {
            continue;
        }

        ScreenPositionComponent* spc = static_cast<ScreenPositionComponent*>(l.linkComponent->GetTargetEntity()->GetComponent(Component::SCREEN_POSITION_COMPONENT));
        if (spc)
        {
            Vector3 positionAndDepth = spc->GetScreenPositionAndDepth();
            if (l.visibilityComponent && l.visibilityComponent->IsEnabled())
            {
                l.control->SetVisibilityFlag(positionAndDepth.z > 0.f && spc->GetCameraViewport().PointInside(Vector2(positionAndDepth.x, positionAndDepth.y)));
                // TODO: skip next steps if invisible?
            }
            if (l.positionComponent && l.positionComponent->IsEnabled())
            {
                l.control->SetPosition(Vector2(positionAndDepth.x, positionAndDepth.y));
            }
            if (l.scaleComponent && l.scaleComponent->IsEnabled())
            {
                if (!FLOAT_EQUAL(positionAndDepth.z, 0.f))
                {
                    const Vector2& factor = l.scaleComponent->GetDepthToScaleFactor();
                    const Vector2& maxScale = l.scaleComponent->GetMaxScale();
                    const Vector2& minScale = l.scaleComponent->GetMinScale();
                    float32 depth = positionAndDepth.z;

                    if (l.scaleComponent->IsUseAbsoluteDepth())
                    {
                        depth = Abs(depth);
                    }

                    Vector2 scale(Clamp(factor.x / depth, minScale.x, maxScale.x),
                                  Clamp(factor.y / depth, minScale.y, maxScale.y));

                    l.control->SetScale(scale);
                }
                else
                {
                    l.control->SetScale(l.scaleComponent->GetMaxScale());
                }
            }
        }
    }
}

UIEntityMarkerSystem::Link* UIEntityMarkerSystem::Find(UIControl* c)
{
    auto it = std::find_if(links.begin(), links.end(), [c](const Link& l) {
        return l.control == c;
    });
    if (it != links.end())
    {
        return &(*it);
    }
    return nullptr;
}
}
