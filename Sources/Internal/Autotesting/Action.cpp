#include "Autotesting/Action.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Autotesting/AutotestingSystem.h"
#include "UI/UIControlSystem.h"
#include "UI/UIScreenManager.h"

namespace DAVA
{

Action::Action() : BaseObject()
    , isExecuted(false)
{
}
Action::~Action()
{
}

// void Action::Load()
// {
// }

void Action::Update(float32 timeElapsed)
{
    if(!isExecuted)
    {
        isExecuted = TestCondition();
    }
}

void Action::Execute()
{
    Logger::Debug("Action::Execute");
    isExecuted = TestCondition();
}

bool Action::TestCondition()
{
    return true;
}

//--------------------------------------------------------------------------

void Action::SetText(const Vector<String> &controlPath, const WideString &text)
{
    //TODO: use specific setters or any other way to get text from custom control
    // otherwise it is supported only for UIStaticText and UITextField classes
    UIControl* control = FindControl(controlPath);
    UIStaticText* staticText = dynamic_cast<UIStaticText*>(control);
    if(staticText)
    {
        staticText->SetText(text);
        Logger::Debug("Action::SetText UIStaticText");
    }
    else
    {
        UITextField* textField = dynamic_cast<UITextField*>(control);
        if(textField)
        {
            textField->SetText(text);
            Logger::Debug("Action::SetText UITextField");
        }
        else
        {
            Logger::Debug("Action::SetText FAILED");
        }
    }
}

WideString Action::GetText(const Vector<String> &controlPath)
{
    //TODO: use specific getters or any other way to get text from custom control
    // otherwise it is supported only for UIStaticText and UITextField classes
    WideString text = L"";
    UIControl* control = FindControl(controlPath);
    UIStaticText* staticText = dynamic_cast<UIStaticText*>(control);
    if(staticText)
    {
        text = staticText->GetText();
    }
    else
    {
        UITextField* textField = dynamic_cast<UITextField*>(control);
        if(textField)
        {
            text = textField->GetText();
        }
    }
    return text;
}

//----------------------------------------------------------------------
// helpers
void Action::ProcessInput(const UIEvent &input)
{
    Logger::Debug("AutotestingSystem::ProcessInput %d phase=%d point=(%f, %f) key=%c",input.tid, input.phase, input.point.x, input.point.y, input.keyChar);

    Vector<UIEvent> emptyTouches;
    Vector<UIEvent> touches;
    touches.push_back(input);
    UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);

    AutotestingSystem::Instance()->OnInput(input);
}

UIControl* Action::FindControl(const Vector<String>& controlPath)
{
    UIControl* control = NULL;
    if(UIScreenManager::Instance()->GetScreen() && (!controlPath.empty()))
    {
        control = UIScreenManager::Instance()->GetScreen()->FindByName(controlPath[0]);
        for(int32 i = 1; i < controlPath.size(); ++i)
        {
            if(!control) break;
            control = control->FindByName(controlPath[i]);
        }
    }
    return control;
}

Vector2 Action::FindControlPosition(const Vector<String>& controlPath)
{
    Vector2 point;
    UIControl* control = FindControl(controlPath);
    if(control)
    {
        point = control->GetPosition(true);
    }
    return point;
}

//----------------------------------------------------------------------
KeyPressAction::KeyPressAction(char16 _keyChar) : Action()
    , keyChar(_keyChar)
{
}

KeyPressAction::~KeyPressAction()
{
}

void KeyPressAction::Execute()
{
    Logger::Debug("KeyPressAction::Execute");
    KeyPress(keyChar);
    Action::Execute();
}

void KeyPressAction::KeyPress(char16 keyChar)
{
    //TODO: KeyPress
    Logger::Debug("KeyPressAction::KeyPress %c", keyChar);
    UIEvent keyPress;
    keyPress.tid = 0;
    keyPress.phase = UIEvent::PHASE_KEYCHAR;
    keyPress.tapCount = 1;

    ProcessInput(keyPress);
}

//----------------------------------------------------------------------

WaitAction::WaitAction(float32 _waitTime) : Action()
    , waitTime(_waitTime)
{

}

WaitAction::~WaitAction()
{

}

void WaitAction::Execute()
{
    Action::Execute();
}

void WaitAction::Update(float32 timeElapsed)
{
    waitTime -= timeElapsed;
    Action::Update(timeElapsed);
}

bool WaitAction::TestCondition()
{
    return (waitTime <= 0.0f);
}

//----------------------------------------------------------------------

WaitForUIAction::WaitForUIAction(const String &_controlName) : Action()
{
    controlPath.push_back(_controlName);
}

WaitForUIAction::WaitForUIAction(const Vector<String> &_controlPath) : Action()
    , controlPath(_controlPath)
{
}

WaitForUIAction::~WaitForUIAction()
{
}

void WaitForUIAction::Execute()
{
    Action::Execute();
}

bool WaitForUIAction::TestCondition()
{
    return (FindControl(controlPath) != NULL);
}

//----------------------------------------------------------------------

SetTextAction::SetTextAction(const String& _controlName, const WideString &_text)
    : Action()
    , text(_text)
{
    controlPath.push_back(_controlName);
}

SetTextAction::SetTextAction(const Vector<String>& _controlPath, const WideString &_text)
    : Action()
    , text(_text)
    , controlPath(_controlPath)
{
}

SetTextAction::~SetTextAction()
{
}

void SetTextAction::Execute()
{
    Logger::Debug("SetTextAction::Execute");
    Action::SetText(controlPath, text);
    Action::Execute();
}
//----------------------------------------------------------------------

// ExecuteYamlAction::ExecuteYamlAction(const String& _yamlPath) : Action()
//     , yamlPath(_yamlPath)
// {
// }
// 
// ExecuteYamlAction::~ExecuteYamlAction()
// {
// }
//
// void ExecuteYamlAction::Load()
// {
// }
//
// void ExecuteYamlAction::Execute()
// {
//     Action::Execute();
// }



//----------------------------------------------------------------------


};

#endif //__DAVAENGINE_AUTOTESTING__