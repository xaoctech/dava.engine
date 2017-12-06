#include "UIAnchorSafeAreaComponent.h"

#include "Engine/Engine.h"
#include "Entity/ComponentManager.h"
#include "UI/UIControl.h"
#include "Reflection/ReflectionRegistrator.h"

ENUM_DECLARE(DAVA::UIAnchorSafeAreaComponent::eInsetType)
{
    ENUM_ADD_DESCR(static_cast<DAVA::int32>(DAVA::UIAnchorSafeAreaComponent::eInsetType::NONE), "NONE");
    ENUM_ADD_DESCR(static_cast<DAVA::int32>(DAVA::UIAnchorSafeAreaComponent::eInsetType::INSET), "INSET");
    ENUM_ADD_DESCR(static_cast<DAVA::int32>(DAVA::UIAnchorSafeAreaComponent::eInsetType::INSET_ONLY_IF_NOTCH), "INSET_ONLY_IF_NOTCH");
    ENUM_ADD_DESCR(static_cast<DAVA::int32>(DAVA::UIAnchorSafeAreaComponent::eInsetType::REVERSE), "REVERSE");
};

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIAnchorSafeAreaComponent)
{
    ReflectionRegistrator<UIAnchorSafeAreaComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIAnchorSafeAreaComponent* o) { o->Release(); })
    .Field("leftSafeInset", &UIAnchorSafeAreaComponent::GetLeftInset, &UIAnchorSafeAreaComponent::SetLeftInset)[M::EnumT<UIAnchorSafeAreaComponent::eInsetType>()]
    .Field("topSafeInset", &UIAnchorSafeAreaComponent::GetTopInset, &UIAnchorSafeAreaComponent::SetTopInset)[M::EnumT<UIAnchorSafeAreaComponent::eInsetType>()]
    .Field("rightSafeInset", &UIAnchorSafeAreaComponent::GetRightInset, &UIAnchorSafeAreaComponent::SetRightInset)[M::EnumT<UIAnchorSafeAreaComponent::eInsetType>()]
    .Field("bottomSafeInset", &UIAnchorSafeAreaComponent::GetBottomInset, &UIAnchorSafeAreaComponent::SetBottomInset)[M::EnumT<UIAnchorSafeAreaComponent::eInsetType>()]
    .End();
}

IMPLEMENT_UI_COMPONENT(UIAnchorSafeAreaComponent);

UIAnchorSafeAreaComponent::UIAnchorSafeAreaComponent()
{
}

UIAnchorSafeAreaComponent::UIAnchorSafeAreaComponent(const UIAnchorSafeAreaComponent& src)
    : leftInset(src.leftInset)
    , topInset(src.topInset)
    , rightInset(src.rightInset)
    , bottomInset(src.bottomInset)
{
}

UIAnchorSafeAreaComponent::~UIAnchorSafeAreaComponent()
{
}

UIAnchorSafeAreaComponent* UIAnchorSafeAreaComponent::Clone() const
{
    return new UIAnchorSafeAreaComponent(*this);
}

UIAnchorSafeAreaComponent::eInsetType UIAnchorSafeAreaComponent::GetLeftInset() const
{
    return leftInset;
}

void UIAnchorSafeAreaComponent::SetLeftInset(eInsetType inset)
{
    leftInset = inset;
    MarkDirty();
}

UIAnchorSafeAreaComponent::eInsetType UIAnchorSafeAreaComponent::GetTopInset() const
{
    return topInset;
}

void UIAnchorSafeAreaComponent::SetTopInset(eInsetType inset)
{
    topInset = inset;
    MarkDirty();
}

UIAnchorSafeAreaComponent::eInsetType UIAnchorSafeAreaComponent::GetRightInset() const
{
    return rightInset;
}

void UIAnchorSafeAreaComponent::SetRightInset(eInsetType inset)
{
    rightInset = inset;
    MarkDirty();
}

UIAnchorSafeAreaComponent::eInsetType UIAnchorSafeAreaComponent::GetBottomInset() const
{
    return bottomInset;
}

void UIAnchorSafeAreaComponent::SetBottomInset(eInsetType inset)
{
    bottomInset = inset;
    MarkDirty();
}

void UIAnchorSafeAreaComponent::MarkDirty()
{
    if (GetControl() != nullptr)
    {
        GetControl()->SetLayoutDirty();
    }
}
}
