#pragma once

#include "UI/Components/UIComponent.h"
#include "Reflection/Reflection.h"
#include "Base/RefPtr.h"
#include "Math/Vector.h"

namespace DAVA
{
class Entity;

class UIEntityMarkerScaleComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UIEntityMarkerScaleComponent, UIComponent);
    DECLARE_UI_COMPONENT(UIEntityMarkerScaleComponent);

public:
    UIEntityMarkerScaleComponent();
    UIEntityMarkerScaleComponent(const UIEntityMarkerScaleComponent& src);
    UIEntityMarkerScaleComponent& operator=(const UIEntityMarkerScaleComponent&) = delete;

    UIEntityMarkerScaleComponent* Clone() const override;

    bool IsEnabled() const;
    void SetEnabled(bool enable);
    bool IsUseAbsoluteDepth() const;
    void SetUseAbsoluteDepth(bool absolute);
    const Vector2& GetDepthToScaleFactor() const;
    void SetDepthToScaleFactor(const Vector2& factor);
    const Vector2& GetMaxScale() const;
    void SetMaxScale(const Vector2& s);
    const Vector2& GetMinScale() const;
    void SetMinScale(const Vector2& s);

protected:
    ~UIEntityMarkerScaleComponent();

private:
    bool enabled = true;
    bool useAbsoluteDepth = true;
    Vector2 depthToScaleFactor = Vector2(1.f, 1.f);
    Vector2 maxScale = Vector2(2.f, 2.f);
    Vector2 minScale = Vector2(.1f, .1f);
};

inline bool UIEntityMarkerScaleComponent::IsEnabled() const
{
    return enabled;
}

inline void UIEntityMarkerScaleComponent::SetEnabled(bool enable)
{
    enabled = enable;
}

inline bool UIEntityMarkerScaleComponent::IsUseAbsoluteDepth() const
{
    return useAbsoluteDepth;
}

inline void UIEntityMarkerScaleComponent::SetUseAbsoluteDepth(bool absolute)
{
    useAbsoluteDepth = absolute;
}

inline const Vector2& UIEntityMarkerScaleComponent::GetDepthToScaleFactor() const
{
    return depthToScaleFactor;
}

inline void UIEntityMarkerScaleComponent::SetDepthToScaleFactor(const Vector2& factor)
{
    depthToScaleFactor = factor;
}

inline const Vector2& UIEntityMarkerScaleComponent::GetMaxScale() const
{
    return maxScale;
}

inline void UIEntityMarkerScaleComponent::SetMaxScale(const Vector2& s)
{
    maxScale = s;
}

inline const Vector2& UIEntityMarkerScaleComponent::GetMinScale() const
{
    return minScale;
}

inline void UIEntityMarkerScaleComponent::SetMinScale(const Vector2& s)
{
    minScale = s;
}
}