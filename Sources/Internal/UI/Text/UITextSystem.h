#pragma once

#include "Base/BaseTypes.h"
#include "UI/UISystem.h"

struct UITextSystemTest;

namespace DAVA
{
class UIControl;
class UITextComponent;
class UITextSystemLink;

/** 
    Text component support system. 
    Manage component internal structure.
    \sa UITextComponent 
*/
class UITextSystem final : public UISystem
{
public:
    UITextSystem() = default;
    ~UITextSystem() override = default;

    void RegisterControl(UIControl* control) override;
    void UnregisterControl(UIControl* control) override;
    void RegisterComponent(UIControl* control, UIComponent* component) override;
    void UnregisterComponent(UIControl* control, UIComponent* component) override;

    void Process(float32 elapsedTime) override;

private:
    void AddLink(UITextComponent* component);
    void RemoveLink(UITextComponent* component);

    Vector<UITextSystemLink*> links;

    friend struct UITextSystemTest;
};
}
