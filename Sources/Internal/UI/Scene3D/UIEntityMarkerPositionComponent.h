#pragma once

#include "UI/Components/UIComponent.h"
#include "Reflection/Reflection.h"
#include "Base/RefPtr.h"

namespace DAVA
{
class Entity;

class UIEntityMarkerPositionComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UIEntityMarkerPositionComponent, UIComponent);
    DECLARE_UI_COMPONENT(UIEntityMarkerPositionComponent);

public:
    UIEntityMarkerPositionComponent();
    UIEntityMarkerPositionComponent(const UIEntityMarkerPositionComponent& src);
    UIEntityMarkerPositionComponent& operator=(const UIEntityMarkerPositionComponent&) = delete;

    UIEntityMarkerPositionComponent* Clone() const override;

    bool IsEnabled() const;
    void SetEnabled(bool enable);

protected:
    ~UIEntityMarkerPositionComponent();

private:
    bool enabled = true;
};

inline bool UIEntityMarkerPositionComponent::IsEnabled() const
{
    return enabled;
}

inline void UIEntityMarkerPositionComponent::SetEnabled(bool enable)
{
    enabled = enable;
}


}