#include "UIAnchorSafeAreaComponent.h"

#include "Engine/Engine.h"
#include "Entity/ComponentManager.h"
#include "UI/UIControl.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIAnchorSafeAreaComponent)
{
    ReflectionRegistrator<UIAnchorSafeAreaComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIAnchorSafeAreaComponent* o) { o->Release(); })
    .Field("leftSafeInset", &UIAnchorSafeAreaComponent::IsUseLeftSafeInset, &UIAnchorSafeAreaComponent::SetUseLeftSafeInset)
    .Field("topSafeInset", &UIAnchorSafeAreaComponent::IsUseTopSafeInset, &UIAnchorSafeAreaComponent::SetUseTopSafeInset)
    .Field("rightSafeInset", &UIAnchorSafeAreaComponent::IsUseRightSafeInset, &UIAnchorSafeAreaComponent::SetUseRightSafeInset)
    .Field("bottomSafeInset", &UIAnchorSafeAreaComponent::IsUseBottomSafeInset, &UIAnchorSafeAreaComponent::SetUseBottomSafeInset)
    .End();
}

IMPLEMENT_UI_COMPONENT(UIAnchorSafeAreaComponent);

UIAnchorSafeAreaComponent::UIAnchorSafeAreaComponent()
{
}

UIAnchorSafeAreaComponent::UIAnchorSafeAreaComponent(const UIAnchorSafeAreaComponent& src)
    : flags(src.flags)
{
}

UIAnchorSafeAreaComponent::~UIAnchorSafeAreaComponent()
{
}

UIAnchorSafeAreaComponent* UIAnchorSafeAreaComponent::Clone() const
{
    return new UIAnchorSafeAreaComponent(*this);
}

bool UIAnchorSafeAreaComponent::IsUseLeftSafeInset() const
{
    return flags.test(FLAG_USE_LEFT_SAFE_INSET);
}

void UIAnchorSafeAreaComponent::SetUseLeftSafeInset(bool use)
{
    SetFlag(FLAG_USE_LEFT_SAFE_INSET, use);
}

bool UIAnchorSafeAreaComponent::IsUseRightSafeInset() const
{
    return flags.test(FLAG_USE_RIGHT_SAFE_INSET);
}

void UIAnchorSafeAreaComponent::SetUseRightSafeInset(bool use)
{
    SetFlag(FLAG_USE_RIGHT_SAFE_INSET, use);
}

bool UIAnchorSafeAreaComponent::IsUseTopSafeInset() const
{
    return flags.test(FLAG_USE_TOP_SAFE_INSET);
}

void UIAnchorSafeAreaComponent::SetUseTopSafeInset(bool use)
{
    SetFlag(FLAG_USE_TOP_SAFE_INSET, use);
}

bool UIAnchorSafeAreaComponent::IsUseBottomSafeInset() const
{
    return flags.test(FLAG_USE_BOTTOM_SAFE_INSET);
}

void UIAnchorSafeAreaComponent::SetUseBottomSafeInset(bool use)
{
    SetFlag(FLAG_USE_BOTTOM_SAFE_INSET, use);
}

void UIAnchorSafeAreaComponent::SetFlag(eFlags flag, bool enabled)
{
    if (flags.test(flag) != enabled)
    {
        flags.set(flag, enabled);
        if (GetControl() != nullptr)
        {
            GetControl()->SetLayoutDirty();
        }
    }
}
}
