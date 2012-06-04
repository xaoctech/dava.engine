#include "Autotesting/AutotestingSystem.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Utils/Utils.h"

namespace DAVA
{

AutotestingSystem::AutotestingSystem() : currentTest(NULL), isRunning(false)
{
    testsParser = new TestsYamlParser();
}

AutotestingSystem::~AutotestingSystem()
{
    for(Deque<Test*>::iterator it = tests.begin(); it != tests.end(); ++it)
    {
        SafeRelease(*it);
    }
    tests.clear();

    SafeRelease(testsParser);
}

void AutotestingSystem::AddTest(Test* test)
{
    SafeRetain(test);
    tests.push_back(test);
}

void AutotestingSystem::AddTestsFromYaml(const String &yamlFilePath)
{
    testsParser->ParseTestsYaml(yamlFilePath);
}

void AutotestingSystem::RunTests()
{
    isRunning = true;
}

void AutotestingSystem::Update(float32 timeElapsed)
{
    if(isRunning)
    {
        //TODO: remove all executed tests?
        if(tests.empty())
        {
            isRunning = false;
            OnTestsFinished();
        }
        else
        {
            // executes at most one command per update
            //TODO: execute simultaneously?
            if(!currentTest)
            {
                currentTest = tests.front();
                currentTest->Execute();
            }
        
            if(currentTest)
            {
                if(!currentTest->IsExecuted())
                {
                    currentTest->Update(timeElapsed);
                }
                else
                {
                    SafeRelease(currentTest);
                    tests.pop_front();
                }
            }
            else
            {
                tests.pop_front();
            }
        }
    }
}

void AutotestingSystem::Draw()
{
    if(!touches.empty())
    {
        for(Map<int32, UIEvent>::iterator it = touches.begin(); it != touches.end(); ++it)
        {
            Vector2 point = it->second.point;
            RenderHelper::Instance()->DrawCircle(point, 20.0f);
        }
    }
}

void AutotestingSystem::OnTestsFinished()
{
    Logger::Debug("AutotestingSystem::OnTestsFinished");
    //TODO: all tests finished. report?
}


void AutotestingSystem::Click(const Vector2 &point, int32 id)
{
    Logger::Debug("AutotestingSystem::Click point=(%f, %f) id=%d", point.x, point.y, id);
    TouchDown(point, id);
    Wait(0.1f);
    TouchUp(id);
}

void AutotestingSystem::Click(const String &controlName, int32 id)
{
    Logger::Debug("AutotestingSystem::Click controlName=%s id=%d", controlName.c_str(), id);
    TouchDown(controlName, id);
    TouchUp(id);
}

void AutotestingSystem::TouchDown(const Vector2 &point, int32 id)
{
    Logger::Debug("AutotestingSystem::TouchDown point=(%f, %f) id=%d", point.x, point.y, id);
    TouchDownTest* touchDownTest = new TouchDownTest(point, id);
    AddTest(touchDownTest);
    SafeRelease(touchDownTest);
}

void AutotestingSystem::TouchDown(const String &controlName, int32 id)
{
    Logger::Debug("AutotestingSystem::TouchDown controlName=%s id=%d", controlName.c_str(), id);
    TouchControlTest* touchDownTest = new TouchControlTest(controlName, id);
    AddTest(touchDownTest);
    SafeRelease(touchDownTest);
}

void AutotestingSystem::TouchUp(int32 id)
{
    Logger::Debug("AutotestingSystem::TouchUp id=%d");
    TouchUpTest* touchUpTest = new TouchUpTest(id);
    AddTest(touchUpTest);
    SafeRelease(touchUpTest);
}

void AutotestingSystem::TouchMove(const Vector2 &toPoint, float32 time, int32 id)
{
    Logger::Debug("AutotestingSystem::TouchMove");
    TouchMoveTest* touchMoveTest = new TouchMoveTest(toPoint, time, id);
    AddTest(touchMoveTest);
    SafeRelease(touchMoveTest);
}

void AutotestingSystem::KeyPress(char16 keyChar)
{
    Logger::Debug("AutotestingSystem::KeyPress %c",keyChar);
    KeyPressTest* keyPressTest = new KeyPressTest(keyChar);
    AddTest(keyPressTest);
    SafeRelease(keyPressTest);
}

void AutotestingSystem::KeyboardInput(const WideString &text)
{
    Logger::Debug("AutotestingSystem::KeyboardInput %s",WStringToString(text).c_str());
    for(int32 i = 0; i < text.size(); ++i)
    {
        KeyPress(text[i]);
    }
}

void AutotestingSystem::SetText(const String &controlName, const WideString &text)
{
    Logger::Debug("AutotestingSystem::SetText controlName=%s text=%s",controlName.c_str(), WStringToString(text).c_str());
    SetTextTest* setTextTest = new SetTextTest(controlName, text);
    AddTest(setTextTest);
    SafeRelease(setTextTest);
}
    
void AutotestingSystem::Wait(float32 time)
{
    Logger::Debug("AutotestingSystem::Wait time=%f", time);
    WaitTest* waitTest = new WaitTest(time);
    AddTest(waitTest);
    SafeRelease(waitTest);
}

void AutotestingSystem::WaitForUI(const String &controlName)
{
    Logger::Debug("AutotestingSystem::WaitForUI controlName=%s", controlName.c_str());
    WaitForUITest* waitForUITest = new WaitForUITest(controlName);
    AddTest(waitForUITest);
    SafeRelease(waitForUITest);
}

void AutotestingSystem::OnInput(const UIEvent &input)
{
    int32 id = input.tid;
    switch(input.phase)
    {
    case UIEvent::PHASE_BEGAN:
        {
            if(!IsTouchDown(id))
            {
                touches[id] = input;
            }
            else
            {
                Logger::Error("AutotestingSystem::OnInput PHASE_BEGAN duplicate touch id=%d",id);
            }
        }
        break;
    case UIEvent::PHASE_MOVE:
        {
            if(!IsTouchDown(id))
            {
                mouseMove = input;
            }
            else
            {
                Logger::Error("AutotestingSystem::OnInput PHASE_MOVE id=%d must be PHASE_DRAG",id);
            }
        }
        break;
    case UIEvent::PHASE_DRAG:
        {
            Map<int32, UIEvent>::iterator findIt = touches.find(id);
            if(findIt != touches.end())
            {
                findIt->second = input;
            }
            else
            {
                Logger::Error("AutotestingSystem::OnInput PHASE_DRAG id=%d must be PHASE_MOVE",id);
            }
        }
        break;
    case UIEvent::PHASE_ENDED:
        {
            Map<int32, UIEvent>::iterator findIt = touches.find(id);
            if(findIt != touches.end())
            {
                touches.erase(findIt);
            }
            else
            {
                Logger::Error("AutotestingSystem::OnInput PHASE_ENDED id=%d not found",id);
            }
        }
        break;
    default:
        //TODO: keyboard input
        break;
    }
}

bool AutotestingSystem::FindTouch(int32 id, UIEvent &touch)
{
    bool isFound = false;
    Map<int32, UIEvent>::iterator findIt = touches.find(id);
    if(findIt != touches.end())
    {
        isFound = true;
        touch = findIt->second;
    }
    return isFound;
}

bool AutotestingSystem::IsTouchDown(int32 id)
{
    return (touches.find(id) != touches.end());
}

};

#endif //__DAVAENGINE_AUTOTESTING__