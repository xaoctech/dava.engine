#pragma once

#include "Base/BaseTypes.h"
#include "UI/UISystem.h"

struct UITextSystemTest;

namespace DAVA
{
class UIControl;
class UITextComponent;

/** 
    Text component support system. 
    Manage component internal structure.
    \sa UITextComponent 
*/
class UITextSystem final : public UISystem
{
public:
    UITextSystem() = default;
    ~UITextSystem() override;

    void RegisterControl(UIControl* control) override;
    void UnregisterControl(UIControl* control) override;
    void RegisterComponent(UIControl* control, UIComponent* component) override;
    void UnregisterComponent(UIControl* control, UIComponent* component) override;

    void Process(float32 elapsedTime) override;
    /** Apply component properties changes to internal TextBlock and UIControlBackground objects. */
    void ApplyData(UITextComponent* component);

private:
    void AddLink(UITextComponent* component);
    void RemoveLink(UITextComponent* component);

    Vector<UITextComponent*> components;
};
}
