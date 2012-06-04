#include "Autotesting/Test.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Autotesting/AutotestingSystem.h"
#include "UI/UIControlSystem.h"
#include "UI/UIScreenManager.h"

namespace DAVA
{

Test::Test() : BaseObject()
    , isExecuted(false)
{
}
Test::~Test()
{
}

void Test::Update(float32 timeElapsed)
{
    if(!isExecuted)
    {
        isExecuted = TestCondition();
    }
}

void Test::Execute()
{
    Logger::Debug("Test::Execute");
    isExecuted = TestCondition();
}

bool Test::TestCondition()
{
    return true;
}

//--------------------------------------------------------------------------

void Test::SetText(const String &controlName, const WideString &text)
{
    Logger::Debug("Test::SetText controlName=%s text=%s",controlName.c_str(), WStringToString(text).c_str());
    //TODO: use specific setters or any other way to get text from custom control
    // otherwise it is supported only for UIStaticText and UITextField classes
    UIControl* control = FindByName(controlName);
    UIStaticText* staticText = dynamic_cast<UIStaticText*>(control);
    if(staticText)
    {
        staticText->SetText(text);
        Logger::Debug("Test::SetText UIStaticText");
    }
    else
    {
        UITextField* textField = dynamic_cast<UITextField*>(control);
        if(textField)
        {
            textField->SetText(text);
            Logger::Debug("Test::SetText UITextField");
        }
        else
        {
            Logger::Debug("Test::SetText FAILED");
        }
    }
}

WideString Test::GetText(const String &controlName)
{
    //TODO: use specific getters or any other way to get text from custom control
    // otherwise it is supported only for UIStaticText and UITextField classes
    WideString text = L"";
    UIControl* control = FindByName(controlName);
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
    Logger::Debug("Test::GetText controlName=%s text=%s", controlName.c_str(), WStringToString(text).c_str());
    return text;
}

//----------------------------------------------------------------------
// helpers
void Test::ProcessInput(const UIEvent &input)
{
    Logger::Debug("AutotestingSystem::ProcessInput %d phase=%d point=(%f, %f) key=%c",input.tid, input.phase, input.point.x, input.point.y, input.keyChar);

    Vector<UIEvent> emptyTouches;
    Vector<UIEvent> touches;
    touches.push_back(input);
    UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);

    AutotestingSystem::Instance()->OnInput(input);
}

UIControl* Test::FindByName(const String& controlName)
{
    UIControl* control = NULL;
    if(UIScreenManager::Instance()->GetScreen())
    {
        control = UIScreenManager::Instance()->GetScreen()->FindByName(controlName);
    }
    return control;
}

Vector2 Test::FindControlPositionByName(const String& controlName)
{
    Vector2 point;
    UIControl* control = FindByName(controlName);
    if(control)
    {
        point = control->GetPosition(true);
    }
    return point;
}



//----------------------------------------------------------------------
KeyPressTest::KeyPressTest(char16 _keyChar) : Test()
    , keyChar(_keyChar)
{
}

KeyPressTest::~KeyPressTest()
{
}

void KeyPressTest::Execute()
{
    Logger::Debug("KeyPressTest::Execute");
    KeyPress(keyChar);
    Test::Execute();
}

void KeyPressTest::KeyPress(char16 keyChar)
{
    //TODO: KeyPress
    Logger::Debug("KeyPressTest::KeyPress %c", keyChar);
    UIEvent keyPress;
    keyPress.tid = 0;
    keyPress.phase = UIEvent::PHASE_KEYCHAR;
    keyPress.tapCount = 1;

    ProcessInput(keyPress);
}

//----------------------------------------------------------------------

WaitTest::WaitTest(float32 _waitTime) : Test()
    , waitTime(_waitTime)
{

}

WaitTest::~WaitTest()
{

}

void WaitTest::Execute()
{
    Test::Execute();
}

void WaitTest::Update(float32 timeElapsed)
{
    waitTime -= timeElapsed;
    Test::Update(timeElapsed);
}

bool WaitTest::TestCondition()
{
    return (waitTime <= 0.0f);
}

//----------------------------------------------------------------------

WaitForUITest::WaitForUITest(const String &_controlName) : Test()
    , controlName(_controlName)
{
}

WaitForUITest::~WaitForUITest()
{
}

void WaitForUITest::Execute()
{
    Logger::Debug("WaitForUITest::Execute");
    Test::Execute();
}

bool WaitForUITest::TestCondition()
{
    return (FindByName(controlName) != NULL);
}

//----------------------------------------------------------------------

SetTextTest::SetTextTest(const String& _controlName, const WideString &_text)
    : Test()
    , controlName(_controlName)
    , text(_text)
{

}

SetTextTest::~SetTextTest()
{
}

void SetTextTest::Execute()
{
    Logger::Debug("SetTextTest::Execute");
    Test::SetText(controlName, text);
    Test::Execute();
}
//----------------------------------------------------------------------

};

#endif //__DAVAENGINE_AUTOTESTING__