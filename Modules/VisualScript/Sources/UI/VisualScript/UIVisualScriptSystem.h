#pragma once

#include <Base/FastName.h>
#include <Base/Vector.h>
#include <UI/UISystem.h>
#include <UI/Events/UIEventsSystem.h>

namespace DAVA
{
class UIVisualScriptComponent;
class VisualScript;

class UIVisualScriptSystem : public UISystem, UIEventsSystem::UIEventProcessorDelegate
{
public:
    ~UIVisualScriptSystem() override;
    void RegisterSystem() override;
    void UnregisterSystem() override;

    void Process(float32 elapsedTime) override;

    bool ProcessEvent(UIControl* control, const FastName& event) override;

protected:
    void RegisterControl(UIControl* control) override;
    void UnregisterControl(UIControl* control) override;
    void RegisterComponent(UIControl* control, UIComponent* component) override;
    void UnregisterComponent(UIControl* control, UIComponent* component) override;

private:
    struct Link
    {
        bool ready = false;
        std::shared_ptr<VisualScript> script;
    };

    void AddLink(UIVisualScriptComponent* c);
    void RemoveLink(UIVisualScriptComponent* c);
    void UpdateVariables(UIVisualScriptComponent* c, Link& link);

    UnorderedMap<UIVisualScriptComponent*, Link> links;
};
} // namespace DAVA
