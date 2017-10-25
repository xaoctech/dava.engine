#pragma once

#include "Base/BaseTypes.h"

#include "Reflection/Reflection.h"
#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UIControl;

class UIInputEventComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UIInputEventComponent, UIComponent);
    DECLARE_UI_COMPONENT(UIInputEventComponent);

public:
    UIInputEventComponent();
    UIInputEventComponent(const UIInputEventComponent& src);

protected:
    virtual ~UIInputEventComponent();

private:
    UIInputEventComponent& operator=(const UIInputEventComponent&) = delete;

public:
    UIInputEventComponent* Clone() const override;

    const FastName& GetOnTouchDownEvent() const;
    void SetOnTouchDownEvent(const FastName& value);

    const FastName& GetOnTouchUpInsideEvent() const;
    void SetOnTouchUpInsideEvent(const FastName& value);

    const FastName& GetOnTouchUpOutsideEvent() const;
    void SetOnTouchUpOutsideEvent(const FastName& value);

    const FastName& GetOnValueChangedEvent() const;
    void SetOnValueChangedEvent(const FastName& value);

    const FastName& GetOnHoverSetEvent() const;
    void SetOnHoverSetEvent(const FastName& value);

    const FastName& GetOnHoverRemovedEvent() const;
    void SetOnHoverRemovedEvent(const FastName& value);

private:
    FastName onTouchDown;
    FastName onTouchUpInside;
    FastName onTouchUpOutside;
    FastName onValueChanged;
    FastName onHoverSet;
    FastName onHoverRemoved;
};
}
