#pragma once

#include "UI/Components/UIComponent.h"
#include <bitset>

namespace DAVA
{
class UIAnchorSafeAreaComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UIAnchorCorrectionComponent, UIComponent);

public:
    DECLARE_UI_COMPONENT(UIAnchorCorrectionComponent);

    UIAnchorSafeAreaComponent();
    UIAnchorSafeAreaComponent(const UIAnchorSafeAreaComponent& src);

    bool IsUseLeftSafeInset() const;
    void SetUseLeftSafeInset(bool use);

    bool IsUseTopSafeInset() const;
    void SetUseTopSafeInset(bool use);

    bool IsUseRightSafeInset() const;
    void SetUseRightSafeInset(bool use);

    bool IsUseBottomSafeInset() const;
    void SetUseBottomSafeInset(bool use);

protected:
    virtual ~UIAnchorSafeAreaComponent();

private:
    UIAnchorSafeAreaComponent& operator=(const UIAnchorSafeAreaComponent&) = delete;

public:
    UIAnchorSafeAreaComponent* Clone() const override;

private:
    enum eFlags
    {
        FLAG_USE_LEFT_SAFE_INSET,
        FLAG_USE_TOP_SAFE_INSET,
        FLAG_USE_RIGHT_SAFE_INSET,
        FLAG_USE_BOTTOM_SAFE_INSET,

        FLAGS_COUNT
    };

    void SetFlag(eFlags flag, bool enabled);

    std::bitset<FLAGS_COUNT> flags;
};
}
