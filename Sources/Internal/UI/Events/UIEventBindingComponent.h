#pragma once

#include "Base/BaseTypes.h"

#include "UI/Components/UIComponent.h"
#include "Input/KeyboardShortcut.h"
#include "Functional/Signal.h"
#include "UIActionMap.h"
#include "UIInputMap.h"
#include "UICommandMap.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class UIControl;

class UIEventBindingComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UIEventBindingComponent, UIComponent);
    IMPLEMENT_UI_COMPONENT(UIEventBindingComponent);

public:
    UIEventBindingComponent();
    UIEventBindingComponent(const UIEventBindingComponent& src);

protected:
    virtual ~UIEventBindingComponent();

private:
    UIEventBindingComponent& operator=(const UIEventBindingComponent&) = delete;

public:
    struct CommandWithParams
    {
        FastName commandName;
        UICommandMap::CommandParams params;
    };

    UIEventBindingComponent* Clone() const override;

    UIActionMap& GetActionMap();
    UnorderedMap<FastName, List<CommandWithParams>>& GetCommands();

    void BindAction(const FastName& eventName, const UIActionMap::SimpleAction& action);
    void BindCommand(const FastName& eventName, const CommandWithParams& cmd);
    void UnbindAction(const FastName& eventName);
    void UnbindAllCommands(const FastName& eventName);

private:
    String GetCommandsAsString() const;
    void SetCommandsFromString(const String& value);

    UnorderedMap<FastName, List<CommandWithParams>> commands;

    UIActionMap actionMap;
};
}
