#include "UI/Events/UIEventsSystem.h"
#include "UI/Events/UIEventBindingComponent.h"
#include "UI/Events/UIEventsSingleComponent.h"
#include "UI/Events/UIInputEventComponent.h"
#include "UI/Input/UIInputSystem.h"
#include "UI/UIControl.h"
#include "UI/UIControlSystem.h"
#include "UI/Flow/UIFlowControllerComponent.h"
#include "UI/Flow/UIFlowStateSystem.h"
#include "UI/Script/UIScriptSystem.h"

namespace DAVA
{
const FastName UIEventsSystem::ACTION_COMPONENT_SELF_ENTITY_NAME("*** Self ***");

UIEventsSystem::UIEventsSystem()
{
}

UIEventsSystem::~UIEventsSystem()
{
}

void UIEventsSystem::RegisterSystem()
{
    GetScene()->AddSingleComponent(std::make_unique<UIEventsSingleComponent>());
}

void UIEventsSystem::UnregisterSystem()
{
    GetScene()->RemoveSingleComponent(GetScene()->GetSingleComponent<UIEventsSingleComponent>());
}

void UIEventsSystem::Process(float32 elapsedTime)
{
    UIEventsSingleComponent* eventsSingle = GetScene()->GetSingleComponent<UIEventsSingleComponent>();
    if (eventsSingle)
    {
        while (!eventsSingle->events.empty())
        {
            DefferedEvent event(eventsSingle->events.front());
            eventsSingle->events.pop_front();
            if (event.broadcast)
            {
                SendBroadcastEvent(event.control.Get(), event.event);
            }
            else
            {
                SendEvent(event.control.Get(), event.event);
            }
        }
    }
}

void UIEventsSystem::RegisterCommands()
{
    auto is = GetScene()->GetInputSystem();
    BindGlobalAction(UIInputSystem::ACTION_FOCUS_LEFT, MakeFunction(is, &UIInputSystem::MoveFocusLeft));
    BindGlobalAction(UIInputSystem::ACTION_FOCUS_RIGHT, MakeFunction(is, &UIInputSystem::MoveFocusRight));
    BindGlobalAction(UIInputSystem::ACTION_FOCUS_UP, MakeFunction(is, &UIInputSystem::MoveFocusUp));
    BindGlobalAction(UIInputSystem::ACTION_FOCUS_DOWN, MakeFunction(is, &UIInputSystem::MoveFocusDown));

    BindGlobalAction(UIInputSystem::ACTION_FOCUS_NEXT, MakeFunction(is, &UIInputSystem::MoveFocusForward));
    BindGlobalAction(UIInputSystem::ACTION_FOCUS_PREV, MakeFunction(is, &UIInputSystem::MoveFocusBackward));
}

UIControl* UIEventsSystem::GetTargetEntity(const FastName& name, UIControl* control)
{
    if (control)
    {
        if (name.empty() || (name == ACTION_COMPONENT_SELF_ENTITY_NAME) || (name == control->GetName()))
        {
            return control;
        }
        return control->FindByName(name);
    }
    return nullptr;
}

void UIEventsSystem::BindGlobalShortcut(const KeyboardShortcut& shortcut, const FastName& eventName)
{
    globalShortcutToEvent.BindEvent(shortcut, eventName);
}

void UIEventsSystem::BindGlobalAction(const FastName& eventName, const UIActionMap::SimpleAction& action)
{
    globalActions.Put(eventName, action);
}

void UIEventsSystem::PerformGlobalShortcut(const KeyboardShortcut& shortcut)
{
    FastName event = globalShortcutToEvent.FindEvent(shortcut);
    if (event.IsValid())
    {
        globalActions.Perform(event);
    }
}

bool UIEventsSystem::SendEvent(UIControl* control, const FastName& event)
{
    DVASSERT(GetScene());

    UIControl* modalControl = GetScene()->GetInputSystem()->GetModalControl();
    UIControl* sceneControl = control;
    bool processed = false;

    while (!processed && sceneControl != nullptr)
    {
        processed = ProcessEventOnContol(sceneControl, event, true);
        sceneControl = (sceneControl == modalControl) ? nullptr : sceneControl->GetParent();
    }

    // Send event to state system
    if (!processed)
    {
        processed = GetScene()->GetSystem<UIFlowStateSystem>()->ProcessEvent(event);
    }
    return processed;
}

bool UIEventsSystem::SendBroadcastEvent(UIControl* control, const FastName& event)
{
    DVASSERT(GetScene());

    bool processed = false;
    for (UIControl* sceneControl : control->GetChildren())
    {
        bool processedChild = ProcessEventOnContol(sceneControl, event, false);
        if (!processedChild)
        {
            SendBroadcastEvent(sceneControl, event);
        }
        processed = processed || processedChild;
    }

    return processed;
}

bool UIEventsSystem::ProcessEventOnContol(UIControl* sceneControl, const FastName& event, bool checkFlow)
{
    bool processed = false;

    // Old action binding components
    processed = ProcessEventOnBinding(sceneControl, event);

    if (!processed)
    {
        processed = GetScene()->GetSystem<UIScriptSystem>()->ProcessEvent(sceneControl, event);
    }

    return processed;
}

bool UIEventsSystem::ProcessEventOnBinding(UIControl* sceneControl, const FastName& event)
{
    bool processed = false;
    // Old action binding components
    auto actions = sceneControl->GetComponent<UIEventBindingComponent>();
    if (actions)
    {
        processed = actions->GetActionMap().Perform(event);
    }
    return processed;
}

void UIEventsSystem::ProcessControlEvent(int32 eventType, const UIEvent* uiEvent, UIControl* control)
{
    UIInputEventComponent* inputEvents = control->GetComponent<UIInputEventComponent>();

    auto scene = control->GetScene();
    if (inputEvents && scene)
    {
        UIEventsSingleComponent* eventsSingle = scene->GetSingleComponent<UIEventsSingleComponent>();
        if (eventsSingle)
        {
            FastName event;
            switch (eventType)
            {
            case UIControl::eEventType::EVENT_TOUCH_DOWN:
                event = inputEvents->GetOnTouchDownEvent();
                break;
            case UIControl::eEventType::EVENT_TOUCH_UP_INSIDE:
                event = inputEvents->GetOnTouchUpInsideEvent();
                break;
            case UIControl::eEventType::EVENT_TOUCH_UP_OUTSIDE:
                event = inputEvents->GetOnTouchUpOutsideEvent();
                break;
            case UIControl::eEventType::EVENT_VALUE_CHANGED:
                event = inputEvents->GetOnValueChangedEvent();
                break;
            case UIControl::eEventType::EVENT_HOVERED_SET:
                event = inputEvents->GetOnHoverSetEvent();
                break;
            case UIControl::eEventType::EVENT_HOVERED_REMOVED:
                event = inputEvents->GetOnHoverRemovedEvent();
                break;
            }
            if (event.IsValid())
            {
                eventsSingle->SendEvent(control, event);
            }
        }
    }
}
}
