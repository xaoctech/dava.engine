#pragma once

#include "Base/RefPtr.h"
#include "Functional/Function.h"
#include "Math/Vector.h"
#include "Reflection/Reflection.h"
#include "UI/Components/UIComponent.h"

namespace DAVA
{
class Entity;
class ScreenPositionComponent;

/** Component for setup synchronization params between UIControl and Entity. */
class UIEntityMarkerComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UIEntityMarkerComponent, UIComponent);
    DECLARE_UI_COMPONENT(UIEntityMarkerComponent);

public:
    /** Describe ordering mode. */
    enum class OrderMode : int32
    {
        NearFront = 0, //!< Marker for near Entity will be on front.
        NearBack //!< marker for near Entity will be on back.
    };

    /** Function declaration using for custom strategies. */
    using CustomStrategy = Function<void(UIControl*, UIEntityMarkerComponent*)>;

    UIEntityMarkerComponent();
    UIEntityMarkerComponent(const UIEntityMarkerComponent& src);
    UIEntityMarkerComponent& operator=(const UIEntityMarkerComponent&) = delete;

    UIEntityMarkerComponent* Clone() const override;

    /** Return pointer to target Entity. */
    Entity* GetTargetEntity() const;
    /** Setup target Entity with specified entity's pointer. */
    void SetTargetEntity(Entity* e);

    /** Return enabled flag. */
    bool IsEnabled() const;
    /** Setup enabled flag. */
    void SetEnabled(bool enable);

    /** Return visibility synchronization flag. */
    bool IsSyncVisibilityEnabled() const;
    /** Setup visibility synchronization flag. */
    void SetSyncVisibilityEnabled(bool enable);

    /** Return position synchronization flag. */
    bool IsSyncPositionEnabled() const;
    /** Setup position synchronization flag. */
    void SetSyncPositionEnabled(bool enable);

    /** Return scale synchronization flag. */
    bool IsSyncScaleEnabled() const;
    /** Setup scale synchronization flag. */
    void SetSyncScaleEnabled(bool enable);
    /** Return scale factor value. */
    const Vector2& GetScaleFactor() const;
    /** Setup scale factor value. New scale will calculated as `factor / distance`. */
    void SetScaleFactor(const Vector2& factor);
    /** Return maximum scale value. */
    const Vector2& GetMaxScale() const;
    /** Setup maximum scale value. */
    void SetMaxScale(const Vector2& s);
    /** Return minimum scale value. */
    const Vector2& GetMinScale() const;
    /** Setup minimum scale value. */
    void SetMinScale(const Vector2& s);

    /** Return order synchronization flag. */
    bool IsSyncOrderEnabled() const;
    /** Setup order synchronization flag. */
    void SetSyncOrderEnabled(bool enable);
    /** Return ordering mode. */
    OrderMode GetOrderMode() const;
    /** Setup ordering mode. */
    void SetOrderMode(OrderMode mode);

    /** Return using custom strategy flag. */
    bool IsUseCustomStrategy() const;
    /** Setup using custom strategy flag. */
    void SetUseCustomStrategy(bool enable);
    /** Return custom strategy function. */
    const CustomStrategy& GetCustomStrategy() const;
    /** Setup custom strategy function. */
    void SetCustomStrategy(const CustomStrategy& fn);

protected:
    ~UIEntityMarkerComponent();

private:
    RefPtr<Entity> targetEntity;
    bool enabled = true;
    // Visibility
    bool syncVisibilityEnabled = false;
    // Position
    bool syncPositionEnabled = false;
    // Scale
    bool syncScaleEnabled = false;
    Vector2 scaleFactor = Vector2(1.f, 1.f);
    Vector2 maxScale = Vector2(2.f, 2.f);
    Vector2 minScale = Vector2(.1f, .1f);
    // Order
    bool syncOrderEnabled = false;
    OrderMode orderMode = OrderMode::NearFront;
    // Custom strategy
    bool useCustomStrategy = false;
    CustomStrategy customStrategy;
};

inline Entity* UIEntityMarkerComponent::GetTargetEntity() const
{
    return targetEntity.Get();
}

inline bool UIEntityMarkerComponent::IsEnabled() const
{
    return enabled;
}

inline bool UIEntityMarkerComponent::IsSyncVisibilityEnabled() const
{
    return syncVisibilityEnabled;
}

inline bool UIEntityMarkerComponent::IsSyncPositionEnabled() const
{
    return syncPositionEnabled;
}

inline bool UIEntityMarkerComponent::IsSyncScaleEnabled() const
{
    return syncScaleEnabled;
}

inline const Vector2& UIEntityMarkerComponent::GetScaleFactor() const
{
    return scaleFactor;
}

inline const Vector2& UIEntityMarkerComponent::GetMaxScale() const
{
    return maxScale;
}

inline const Vector2& UIEntityMarkerComponent::GetMinScale() const
{
    return minScale;
}

inline bool UIEntityMarkerComponent::IsSyncOrderEnabled() const
{
    return syncOrderEnabled;
}

inline UIEntityMarkerComponent::OrderMode UIEntityMarkerComponent::GetOrderMode() const
{
    return orderMode;
}

inline bool UIEntityMarkerComponent::IsUseCustomStrategy() const
{
    return useCustomStrategy;
}

inline const UIEntityMarkerComponent::CustomStrategy& UIEntityMarkerComponent::GetCustomStrategy() const
{
    return customStrategy;
}
}
