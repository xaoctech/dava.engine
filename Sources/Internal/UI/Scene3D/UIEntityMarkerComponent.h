#pragma once

#include "UI/Components/UIComponent.h"
#include "Reflection/Reflection.h"
#include "Base/RefPtr.h"

namespace DAVA
{
class Entity;

class UIEntityMarkerComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UIEntityMarkerComponent, UIComponent);
    DECLARE_UI_COMPONENT(UIEntityMarkerComponent);

public:
    UIEntityMarkerComponent();
    UIEntityMarkerComponent(const UIEntityMarkerComponent& src);
    UIEntityMarkerComponent& operator=(const UIEntityMarkerComponent&) = delete;

    UIEntityMarkerComponent* Clone() const override;

    Entity* GetTargetEntity() const;
    void SetTargetEntity(Entity* e);

protected:
    ~UIEntityMarkerComponent();

private:
    RefPtr<Entity> targetEntity; 
};

inline Entity* UIEntityMarkerComponent::GetTargetEntity() const
{
    return targetEntity.Get();
}

inline void UIEntityMarkerComponent::SetTargetEntity(Entity* e)
{
    targetEntity = e;
}


}