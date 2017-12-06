#pragma once

#include "UI/Components/UIComponent.h"
#include "Reflection/Reflection.h"
#include "Base/RefPtr.h"

namespace DAVA
{
class Entity;

class UIEntityMarkerVisibilityComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UIEntityMarkerVisibilityComponent, UIComponent);
    DECLARE_UI_COMPONENT(UIEntityMarkerVisibilityComponent);

public:
    UIEntityMarkerVisibilityComponent();
    UIEntityMarkerVisibilityComponent(const UIEntityMarkerVisibilityComponent& src);
    UIEntityMarkerVisibilityComponent& operator=(const UIEntityMarkerVisibilityComponent&) = delete;

    UIEntityMarkerVisibilityComponent* Clone() const override;

    bool IsEnabled() const;
    void SetEnabled(bool enable);

protected:
    ~UIEntityMarkerVisibilityComponent();

private:
    bool enabled = true;
};

inline bool UIEntityMarkerVisibilityComponent::IsEnabled() const
{
    return enabled;
}

inline void UIEntityMarkerVisibilityComponent::SetEnabled(bool enable)
{
    enabled = enable;
}


}