#include "Autotesting/AutotestingSystem.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Utils/Utils.h"
#include "Render/RenderHelper.h"

namespace DAVA
{

AutotestingSystem::AutotestingSystem() : currentAction(NULL), isRunning(false), isInit(false)
{
}

AutotestingSystem::~AutotestingSystem()
{
    for(Deque<Action*>::iterator it = actions.begin(); it != actions.end(); ++it)
    {
        SafeRelease(*it);
    }
    actions.clear();
}

void AutotestingSystem::OnAppStarted()
{
    if(!isInit)
    {
        //check if autotesting.yaml exists, run if exists
        String yamlFilePath = "~res:/Tests/autotesting.yaml";
        YamlParser* parser = YamlParser::Create(yamlFilePath);
        if(parser)
        {
            YamlNode* rootNode = parser->GetRootNode();
            if(rootNode)
            {
                String testName = "";
                YamlNode* testNameNode = rootNode->Get("testName");
                if(testNameNode)
                {
                    testName = testNameNode->AsString();
                }
                Init(testName);

                YamlNode* actionsNode = rootNode->Get("actions");
                AddActionsFromYamlNode(actionsNode);

                AutotestingSystem::Instance()->RunTests();
            }
            else
            {
                Logger::Error("AutotestingSystem::AddActionsFromYaml %s failed - no root node", yamlFilePath.c_str());
            }
        }
        SafeRelease(parser);
    }
}

void AutotestingSystem::Init(const String &_testName)
{
    if(!isInit)
    {
        isInit = true;
        testName = _testName;
    }
}

void AutotestingSystem::AddAction(Action* action)
{
    if(!isInit) return;

    SafeRetain(action);
    actions.push_back(action);
}

void AutotestingSystem::AddActionsFromYaml(const String &yamlFilePath)
{
    if(!isInit) return;

    YamlParser* parser = YamlParser::Create(yamlFilePath);
    if(parser)
    {
        YamlNode* rootNode = parser->GetRootNode();
        if(rootNode)
        {
            YamlNode* actionsNode = rootNode->Get("actions");
            AddActionsFromYamlNode(actionsNode);
        }
        else
        {
            Logger::Error("AutotestingSystem::AddActionsFromYaml %s failed - no root node", yamlFilePath.c_str());
        }
    }
    SafeRelease(parser);
}

void AutotestingSystem::AddActionsFromYamlNode(YamlNode* actionsNode)
{
    if (!isInit) return;

    if(actionsNode)
    {
        Vector<YamlNode*> actionNodes = actionsNode->AsVector();
        for(int32 i = 0; i < actionNodes.size(); ++i)
        {
            YamlNode* actionNode = actionNodes[i];
            YamlNode* actionNameNode = actionNodes[i]->Get("action");
            if(actionNode && actionNameNode)
            {                        
                String actionName = actionNameNode->AsString();
                Logger::Debug("AutotestingSystem::AddActionsFromYamlNode action=%s", actionName.c_str());
                if(actionName == "ExecuteYaml")
                {
                    YamlNode* pathNode = actionNode->Get("path");
                    if(pathNode)
                    {
                        AddActionsFromYaml(pathNode->AsString());
                    }
                    else
                    {
                        Logger::Error("AutotestingSystem::AddActionsFromYamlNode action %s ignored", actionName.c_str());
                    }
                }
                else if(actionName == "Click")
                {
                    YamlNode* idNode = actionNode->Get("id");
                    YamlNode* pointNode = actionNode->Get("point");
                    if(pointNode)
                    {
                        if(idNode)
                        {
                            Click(pointNode->AsVector2(), idNode->AsInt());
                        }
                        else
                        {
                            Click(pointNode->AsVector2());
                        }
                    }
                    else
                    {
                        YamlNode* controlPathNode = actionNode->Get("controlPath");
                        if(controlPathNode)
                        {
                            if(idNode)
                            {
                                Click(ParseControlPath(controlPathNode), idNode->AsInt());
                            }
                            else
                            {
                                Click(ParseControlPath(controlPathNode));
                            }
                        }
                        else
                        {
                            Logger::Error("AutotestingSystem::AddActionsFromYamlNode action %s ignored", actionName.c_str());
                        }
                    }
                }
                else if(actionName == "TouchDown")
                {
                    YamlNode* idNode = actionNode->Get("id");
                    YamlNode* pointNode = actionNode->Get("point");
                    if(pointNode)
                    {
                        if(idNode)
                        {
                            TouchDown(pointNode->AsVector2(), idNode->AsInt());
                        }
                        else
                        {
                            TouchDown(pointNode->AsVector2());
                        }
                    }
                    else
                    {
                        YamlNode* controlPathNode = actionNode->Get("controlPath");
                        if(controlPathNode)
                        {
                            if(idNode)
                            {
                                TouchDown(ParseControlPath(controlPathNode), idNode->AsInt());
                            }
                            else
                            {
                                TouchDown(ParseControlPath(controlPathNode));
                            }
                        }
                        else
                        {
                            Logger::Error("AutotestingSystem::AddActionsFromYamlNode action %s ignored", actionName.c_str());
                        }
                    }
                }
                else if(actionName == "TouchUp")
                {
                    YamlNode* idNode = actionNode->Get("id");
                    if(idNode)
                    {
                        TouchUp(idNode->AsInt());
                    }
                    else
                    {
                        TouchUp();
                    }
                }
                else if(actionName == "TouchMove")
                {
                    YamlNode* idNode = actionNode->Get("id");
                    YamlNode* pointNode = actionNode->Get("point");
                    YamlNode* timeNode = actionNode->Get("time");
                    if(pointNode)
                    {
                        float32 time = 0.0f;
                        if(timeNode)
                        {
                            time = timeNode->AsFloat();
                        }
                        if(idNode)
                        {
                            TouchMove(pointNode->AsVector2(), time, idNode->AsInt());
                        }
                        else
                        {
                            TouchMove(pointNode->AsVector2(), time);
                        }
                    }
                    else
                    {
                        Logger::Error("AutotestingSystem::AddActionsFromYamlNode action %s ignored", actionName.c_str());
                    }
                }
                else if(actionName == "SetText")
                {
                    YamlNode* controlPathNode = actionNode->Get("controlPath");
                    YamlNode* textNode = actionNode->Get("text");
                    if(controlPathNode)
                    {
                        if(textNode)
                        {
                            SetText(ParseControlPath(controlPathNode), textNode->AsWString());
                        }
                        else
                        {
                            SetText(ParseControlPath(controlPathNode), L"");
                        }
                    }
                    else
                    {
                        Logger::Error("AutotestingSystem::AddActionsFromYamlNode action %s ignored", actionName.c_str());
                    }
                }
                else if(actionName == "Wait")
                {
                    YamlNode* timeNode = actionNode->Get("time");
                    if(timeNode)
                    {
                        Wait(timeNode->AsFloat());
                    }
                    else
                    {
                        Logger::Warning("AutotestingSystem::AddActionsFromYamlNode action %s ignored", actionName.c_str());
                    }
                }
                else if(actionName == "WaitForUI")
                {
                    YamlNode* controlPathNode = actionNode->Get("controlPath");
                    if(controlPathNode)
                    {
                        WaitForUI(ParseControlPath(controlPathNode));
                    }
                    else
                    {
                        Logger::Error("AutotestingSystem::AddActionsFromYamlNode action %s ignored", actionName.c_str());
                    }
                }
                else if(actionName == "KeyPress")
                {
                    YamlNode* keyNode = actionNode->Get("key");
                    if(keyNode)
                    {
                        //TODO: test conversion from int to char16
                        KeyPress((char16)keyNode->AsInt());
                    }
                    else
                    {
                        Logger::Error("AutotestingSystem::AddActionsFromYamlNode action %s ignored", actionName.c_str());
                    }
                }
                else if(actionName == "KeyboardInput")
                {
                    YamlNode* textNode = actionNode->Get("text");
                    if(textNode)
                    {
                        KeyboardInput(textNode->AsWString());
                    }
                    else
                    {
                        Logger::Error("AutotestingSystem::AddActionsFromYamlNode action %s ignored", actionName.c_str());
                    }
                }
                else if(actionName == "Assert")
                {
                    YamlNode* expectedNode = actionNode->Get("expected");
                    YamlNode* actualNode = actionNode->Get("actual");
                    if(expectedNode && actualNode)
                    {
                        YamlNode* expectedGetterNode = expectedNode->Get("getter");
                        YamlNode* actualGetterNode = actualNode->Get("getter");

                        if(expectedGetterNode && actualGetterNode)
                        {
                            String expectedGetterName = expectedGetterNode->AsString();
                            String actualGetterName = actualGetterNode->AsString();

                            YamlNode* expectedControlPathNode = expectedNode->Get("controlPath");
                            YamlNode* actualControlPathNode = actualNode->Get("controlPath");
                            if(expectedGetterName == "GetText")
                            {
                                if(actualGetterName == "GetText")
                                {
                                    AssertText(ParseControlPath(expectedControlPathNode), ParseControlPath(actualControlPathNode));
                                }
                                else
                                {
                                    //TODO: other supported actual getters for expected getter "GetText"
                                    Logger::Error("AutotestingSystem::AddActionsFromYamlNode action %s ignored: wrong actual %s for expected %s", actionName.c_str(), actualGetterName.c_str(), expectedGetterName.c_str());
                                }
                            }
                            else if(expectedGetterName == "FindControl")
                            {
                                if(actualGetterName == "FindControl")
                                {
                                    AssertBool(ParseControlPath(expectedControlPathNode), ParseControlPath(actualControlPathNode));
                                }
                                else
                                {
                                    //TODO: other supported actual getters for expected getter "FindControl"
                                    Logger::Error("AutotestingSystem::AddActionsFromYamlNode action %s ignored: wrong actual %s for expected %s", actionName.c_str(), actualGetterName.c_str(), expectedGetterName.c_str());
                                }
                            }
                        }
                        else if(actualGetterNode)
                        {
                            String actualGetterName = actualGetterNode->AsString();
                            YamlNode* actualControlPathNode = actualNode->Get("controlPath");
                            if(actualGetterName == "GetText")
                            {
                                AssertText(expectedNode->AsWString(), ParseControlPath(actualControlPathNode));
                            }
                            else if(actualGetterName == "FindControl")
                            {
                                AssertBool(expectedNode->AsBool(), ParseControlPath(actualControlPathNode));
                            }
                        }
                        else
                        {
                            Logger::Error("AutotestingSystem::AddActionsFromYamlNode action %s ignored: no actual getter", actionName.c_str());
                        }
                    }
                    else
                    {
                        Logger::Error("AutotestingSystem::AddActionsFromYamlNode action %s ignored: no expected or actual", actionName.c_str());
                    }
                }
                else
                {
                    //TODO: other asserts, getters

                    Logger::Error("AutotestingSystem::AddActionsFromYamlNode action %s ignored", actionName.c_str());
                }
            }
            else
            {
                Logger::Warning("AutotestingSystem::AddActionsFromYamlNode action ignored");
            }
        }
    }
    else
    {
        Logger::Error("AutotestingSystem::AddActionsFromYamlNode failed - no actions node");
    }
}

Vector<String> AutotestingSystem::ParseControlPath(YamlNode* controlPathNode)
{
    Vector<String> controlPath;
    if(controlPathNode)
    {
        Vector<YamlNode*> controlPathNodes = controlPathNode->AsVector();
        if(controlPathNodes.empty())
        {
            controlPath.push_back(controlPathNode->AsString());
        }
        else
        {
            for(int32 i = 0; i < controlPathNodes.size(); ++i)
            {
                YamlNode* controlPathPartNode = controlPathNodes[i];
                if(controlPathPartNode)
                {
                    controlPath.push_back(controlPathPartNode->AsString());
                }
                else
                {
                    Logger::Error("AutotestingSystem::ParseControlPath part failed");
                }
            }
        }
    }
    else
    {
        Logger::Error("AutotestingSystem::ParseControlPath failed");
    }
    return controlPath;
}

void AutotestingSystem::RunTests()
{
    if(!isInit) return;
    isRunning = true;
}

void AutotestingSystem::Update(float32 timeElapsed)
{
    if(!isInit) return;

    if(isRunning)
    {
        //TODO: remove all executed actions?
        if(actions.empty())
        {
            isRunning = false;
            OnTestsFinished();
        }
        else
        {
            // executes at most one command per update
            //TODO: execute simultaneously?
            if(!currentAction)
            {
                currentAction = actions.front();
                currentAction->Execute();
            }
        
            if(currentAction)
            {
                if(!currentAction->IsExecuted())
                {
                    currentAction->Update(timeElapsed);
                }
                else
                {
                    SafeRelease(currentAction);
                    actions.pop_front();
                }
            }
            else
            {
                actions.pop_front();
            }
        }
    }
}

void AutotestingSystem::Draw()
{
    if(!isInit) return;

    if(!touches.empty())
    {
        for(Map<int32, UIEvent>::iterator it = touches.begin(); it != touches.end(); ++it)
        {
            Vector2 point = it->second.point;
            RenderHelper::Instance()->DrawCircle(point, 25.0f);
        }
    }
    RenderHelper::Instance()->DrawCircle(GetMousePosition(), 15.0f);
}

void AutotestingSystem::OnTestsFinished()
{
    Logger::Debug("AutotestingSystem::OnTestsFinished");
    //TODO: all actions finished. report?
}


void AutotestingSystem::Click(const Vector2 &point, int32 id)
{
    TouchDown(point, id);
    Wait(0.1f);
    TouchUp(id);
}

void AutotestingSystem::Click(const String &controlName, int32 id)
{
    TouchDown(controlName, id);
    TouchUp(id);
}

void AutotestingSystem::Click(const Vector<String> &controlPath, int32 id)
{
    TouchDown(controlPath, id);
    TouchUp(id);
}

void AutotestingSystem::TouchDown(const Vector2 &point, int32 id)
{
    TouchDownAction* touchDownAction = new TouchDownAction(point, id);
    AddAction(touchDownAction);
    SafeRelease(touchDownAction);
}

void AutotestingSystem::TouchDown(const String &controlName, int32 id)
{
    TouchDownControlAction* touchDownAction = new TouchDownControlAction(controlName, id);
    AddAction(touchDownAction);
    SafeRelease(touchDownAction);
}

void AutotestingSystem::TouchDown(const Vector<String> &controlPath, int32 id)
{
    TouchDownControlAction* touchDownAction = new TouchDownControlAction(controlPath, id);
    AddAction(touchDownAction);
    SafeRelease(touchDownAction);
}

void AutotestingSystem::TouchUp(int32 id)
{
    TouchUpAction* touchUpAction = new TouchUpAction(id);
    AddAction(touchUpAction);
    SafeRelease(touchUpAction);
}

void AutotestingSystem::TouchMove(const Vector2 &point, float32 time, int32 id)
{
    TouchMoveAction* touchMoveAction = new TouchMoveAction(point, time, id);
    AddAction(touchMoveAction);
    SafeRelease(touchMoveAction);
}

void AutotestingSystem::TouchMove(const String &controlName, float32 time, int32 id)
{
    TouchMoveControlAction* touchMoveAction = new TouchMoveControlAction(controlName, time, id);
    AddAction(touchMoveAction);
    SafeRelease(touchMoveAction);
}

void AutotestingSystem::TouchMove(const Vector<String> &controlPath, float32 time, int32 id)
{
    TouchMoveControlAction* touchMoveAction = new TouchMoveControlAction(controlPath, time, id);
    AddAction(touchMoveAction);
    SafeRelease(touchMoveAction);
}

void AutotestingSystem::KeyPress(char16 keyChar)
{
    KeyPressAction* keyPressAction = new KeyPressAction(keyChar);
    AddAction(keyPressAction);
    SafeRelease(keyPressAction);
}

void AutotestingSystem::KeyboardInput(const WideString &text)
{
    for(int32 i = 0; i < text.size(); ++i)
    {
        KeyPress(text[i]);
    }
}

void AutotestingSystem::SetText(const String &controlName, const WideString &text)
{
    SetTextAction* setTextAction = new SetTextAction(controlName, text);
    AddAction(setTextAction);
    SafeRelease(setTextAction);
}

void AutotestingSystem::SetText(const Vector<String> &controlPath, const WideString &text)
{
    SetTextAction* setTextAction = new SetTextAction(controlPath, text);
    AddAction(setTextAction);
    SafeRelease(setTextAction);
}
    
void AutotestingSystem::Wait(float32 time)
{
    WaitAction* waitAction = new WaitAction(time);
    AddAction(waitAction);
    SafeRelease(waitAction);
}

void AutotestingSystem::WaitForUI(const String &controlName)
{
    WaitForUIAction* waitForUIAction = new WaitForUIAction(controlName);
    AddAction(waitForUIAction);
    SafeRelease(waitForUIAction);
}

void AutotestingSystem::WaitForUI(const Vector<String> &controlPath)
{
    WaitForUIAction* waitForUIAction = new WaitForUIAction(controlPath);
    AddAction(waitForUIAction);
    SafeRelease(waitForUIAction);
}

void AutotestingSystem::AssertText(const WideString &expected, const Vector<String> &controlPath)
{
    VTAssertAction* assertTextAction = new VTAssertAction();
    
    VariantType expectedValue;
    expectedValue.SetWideString(expected);
    Getter* expectedGetter = new Getter(expectedValue);
    assertTextAction->SetExpectedGetter(expectedGetter);
    SafeRelease(expectedGetter);

    ControlTextGetter* actualGetter = new ControlTextGetter(controlPath);
    assertTextAction->SetActualGetter(actualGetter);
    SafeRelease(actualGetter);

    AddAction(assertTextAction);
    SafeRelease(assertTextAction);
}

void AutotestingSystem::AssertText(const Vector<String> &expectedControlPath, const Vector<String> &actualControlPath)
{
    VTAssertAction* assertTextAction = new VTAssertAction();
    
    ControlTextGetter* expectedGetter = new ControlTextGetter(expectedControlPath);
    assertTextAction->SetActualGetter(expectedGetter);
    SafeRelease(expectedGetter);

    ControlTextGetter* actualGetter = new ControlTextGetter(actualControlPath);
    assertTextAction->SetActualGetter(actualGetter);
    SafeRelease(actualGetter);

    AddAction(assertTextAction);
    SafeRelease(assertTextAction);
}
    
void AutotestingSystem::AssertBool(bool expected, const Vector<String> &controlPath)
{
    VTAssertAction* assertBoolAction = new VTAssertAction();
    
    VariantType expectedValue;
    expectedValue.SetBool(expected);
    Getter* expectedGetter = new Getter(expectedValue);
    assertBoolAction->SetExpectedGetter(expectedGetter);
    SafeRelease(expectedGetter);

    ControlTextGetter* actualGetter = new ControlTextGetter(controlPath);
    assertBoolAction->SetActualGetter(actualGetter);
    SafeRelease(actualGetter);

    AddAction(assertBoolAction);
    SafeRelease(assertBoolAction);
}

void AutotestingSystem::AssertBool(const Vector<String> &expectedControlPath, const Vector<String> &actualControlPath)   
{
    VTAssertAction* assertBoolAction = new VTAssertAction();
    
    ControlBoolGetter* expectedGetter = new ControlBoolGetter(expectedControlPath);
    assertBoolAction->SetActualGetter(expectedGetter);
    SafeRelease(expectedGetter);

    ControlBoolGetter* actualGetter = new ControlBoolGetter(actualControlPath);
    assertBoolAction->SetActualGetter(actualGetter);
    SafeRelease(actualGetter);

    AddAction(assertBoolAction);
    SafeRelease(assertBoolAction);
}

void AutotestingSystem::OnTestAssert(const String & text, bool isPassed)
{
    //TODO: report into database
    if(isPassed)
    {
        Logger::Debug("AutotestingSystem::OnTestAssert %s PASSED", text.c_str());
    }
    else
    {
        Logger::Debug("AutotestingSystem::OnTestAssert %s FAILED", text.c_str());
    }
}

void AutotestingSystem::OnInput(const UIEvent &input)
{
    int32 id = input.tid;
    switch(input.phase)
    {
    case UIEvent::PHASE_BEGAN:
        {
            mouseMove = input;
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
            mouseMove = input;
            if(IsTouchDown(id))
            {
                Logger::Error("AutotestingSystem::OnInput PHASE_MOVE id=%d must be PHASE_DRAG",id);
            }
        }
        break;
    case UIEvent::PHASE_DRAG:
        {
            mouseMove = input;
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
            mouseMove = input;
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