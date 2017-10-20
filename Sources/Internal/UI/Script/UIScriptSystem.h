#pragma once

#include "Base/FastName.h"
#include "Base/Vector.h"
#include "UI/UISystem.h"

namespace DAVA
{
class UIScriptComponent;
class UIScriptComponentController;

class UIScriptSystem : public UISystem
{
public:
    ~UIScriptSystem() override;

    void SetPauseProcessing(bool value);
    bool IsPauseProcessing() const;

    bool ProcessEvent(UIControl* control, const FastName& event);
    void Process(float32 elapsedTime) override;
    UIScriptComponentController* GetController(UIScriptComponent* component) const;

protected:
    void RegisterControl(UIControl* control) override;
    void UnregisterControl(UIControl* control) override;
    void RegisterComponent(UIControl* control, UIComponent* component) override;
    void UnregisterComponent(UIControl* control, UIComponent* component) override;

private:
    struct ScriptLink
    {
        UIScriptComponent* component = nullptr;
        std::shared_ptr<UIScriptComponentController> controller;
    };

    void AddScriptLink(UIScriptComponent* component);
    void UpdateController(DAVA::UIScriptSystem::ScriptLink& l);
    void RemoveScriptLink(UIScriptComponent* component);

    Vector<ScriptLink> links;
    bool pauseProcessing = false;
};

inline bool UIScriptSystem::IsPauseProcessing() const
{
    return pauseProcessing;
}
}
