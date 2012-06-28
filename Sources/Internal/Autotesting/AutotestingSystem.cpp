#include "Autotesting/AutotestingSystem.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Utils/Utils.h"
#include "Render/RenderHelper.h"

#include "Core/Core.h"

#include "FileSystem/FileList.h"

namespace DAVA
{

AutotestingSystem::AutotestingSystem() : currentAction(NULL)
    , isInit(false)
    , isRunning(false)
    , projectName("dava.framework")
    , testsId(0)
    , testsDate(0)
    , testIndex(0)
    , testName("")
    , testFileName("")
    , testFilePath("")
    , dbClient(NULL)
#ifdef __DAVAENGINE_AUTOTESTING_FILE__
    , testReportsFolder("")
    , reportFile(NULL)
#endif
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
        // get files list for ~res:/Autotesting/Tests
        FileList fileList("~res:/Autotesting/Tests");
        int32 fileListSize = fileList.GetCount();
        if(fileListSize == 0)
        {
            return;
        }
        
        // read current test index and autotesting id from ~doc:/autotesting.archive
        KeyedArchive* autotestingArchive = new KeyedArchive();

        if(autotestingArchive->Load("~doc:/autotesting/autotesting.archive"))
        {
            testIndex = autotestingArchive->GetInt32("index");
            VariantType* vt = autotestingArchive->GetVariant("id");
            if(vt && vt->type == VariantType::TYPE_UINT32)
            {
                testsId = vt->AsUInt32();
            }
        }

        int32 autotestingId = 1;

        //TODO: read autotesting id from ~res:/Autotesting/autotesting.archive
        File* file = File::Create("~res:/Autotesting/id.txt", File::OPEN | File::READ);
        if(file)
        {
            char tempBuf[1024];
            file->ReadLine(tempBuf, 1024);
            sscanf(tempBuf, "%d", &autotestingId);
            
            if(!file->IsEof())
            {
                char tempBuf[1024];
                file->ReadLine(tempBuf, 1024);
                sscanf(tempBuf, "%d", &testsDate);
            }
            
            if(!file->IsEof())
            {
                char projectCharName[128];
                file->ReadLine(tempBuf, 1024);
                sscanf(tempBuf, "%s", projectCharName);
                SetProjectName(projectCharName);
            }
        }
        SafeRelease(file);
        
        if(!ConnectToDB())
        {
            return;
        }
        
        // compare ids
        if(testsId != autotestingId)
        {
            testIndex = 0;
            testsId = autotestingId;
        }
        
        int32 indexInFileList = testIndex;
        // skip directories
        for(int32 i = 0; (i <= indexInFileList) && (i < fileListSize); ++i)
        {
            if((fileList.IsDirectory(i)) || (fileList.IsNavigationDirectory(i))) indexInFileList++;
        }

        if(indexInFileList < fileListSize)
        {
            testFilePath = fileList.GetPathname(indexInFileList);
            testFileName = fileList.GetFilename(indexInFileList);

            YamlParser* parser = YamlParser::Create(testFilePath);
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
                    OnError(Format("parsing %s failed - no root node", testFilePath.c_str()));
                }
            }
            SafeRelease(parser);
        }

        
        if(indexInFileList == (fileListSize - 1))
        {
            // last file - reset id and index
            autotestingArchive->SetUInt32("id", 0);
            autotestingArchive->SetInt32("index", 0);
        }
        else
        {
            // save next index and autotesting id
            autotestingArchive->SetUInt32("id", testsId);
            autotestingArchive->SetInt32("index", (testIndex + 1));
        }
        autotestingArchive->Save("~doc:/autotesting/autotesting.archive");
    }
}
    
void AutotestingSystem::OnAppFinished()
{
    if(dbClient)
    {
        dbClient->Disconnect();
        SafeRelease(dbClient);
    }
}

void  AutotestingSystem::SetProjectName(const String &_projectName)
{
    projectName = _projectName;
}
    
void AutotestingSystem::Init(const String &_testName)
{
    if(!isInit)
    {
#ifdef __DAVAENGINE_AUTOTESTING_FILE__
        testReportsFolder = "~doc:/autotesting";
        FileSystem::Instance()->CreateDirectory(FileSystem::Instance()->SystemPathForFrameworkPath(testReportsFolder), true);
        reportFile = DAVA::File::Create(Format("%s/autotesting.report",testReportsFolder.c_str()), DAVA::File::CREATE|DAVA::File::WRITE);
#endif

        isInit = true;
        testName = _testName;
    }
}

bool AutotestingSystem::ConnectToDB()
{
    DVASSERT(NULL == dbClient);
    
    dbClient = MongodbClient::Create(AUTOTESTING_DB_IP, AUTOTESTING_DB_PORT);
    if(dbClient)
    {
        dbClient->SetDatabaseName(AUTOTESTING_DB_NAME);
        dbClient->SetCollectionName(projectName);
    }
    
    return (NULL != dbClient);
}
    
void AutotestingSystem::AddTestResult(const String &text, bool isPassed)
{
    testResults.push_back(std::pair< String, bool >(text, isPassed));
}

void AutotestingSystem::SaveTestToDB()
{
    Logger::Debug("AutotestingSystem::SaveTestToDB");
    
    String testAndFileName = Format("%s (%s)", testName.c_str(), testFileName.c_str());
    
    String testsName = Format("%u",testsDate);
    
    MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
    bool isFound = dbClient->FindObjectByKey(testsName, dbUpdateObject);
    if(!isFound)
    {
        dbUpdateObject->SetObjectName(testsName);
    }
    dbUpdateObject->LoadData();
    
    KeyedArchive* dbUpdateData = dbUpdateObject->GetData();
    
    KeyedArchive* platformArchive = NULL;
    
    String logKey = "log";
    KeyedArchive* logArchive = NULL;
    
    KeyedArchive* testArchive = NULL;
    
    String testResultsKey = "TestResults";
    KeyedArchive* testResultsArchive = NULL;
    
    bool isTestPassed = true;
    for(int32 i = 0; i < testResults.size(); ++i)
    {
        if(!testResults[i].second)
        {
            isTestPassed = false;
            break;
        }
    }
    
    bool isTestSuitePassed = isTestPassed;
    
    // find platform object
    if(isFound)
    {
        //found database object
        
        // find platform object
        platformArchive = SafeRetain(dbUpdateData->GetArchive(AUTOTESTING_PLATFORM_NAME, NULL));
        
        if(platformArchive)
        {
            // found platform object
            
            // find log object
            logArchive = SafeRetain(platformArchive->GetArchive(logKey, NULL));
            
            if(logArchive)
            {
                // found log object
                
                // find test object
                testArchive = SafeRetain(logArchive->GetArchive(testName, NULL));
                if(testArchive)
                {
                    // found test object
                    
                    isTestPassed = (testArchive->GetInt32("Success") == 1);
                    
                    // find test results
                    testResultsArchive = SafeRetain(testArchive->GetArchive(testResultsKey));
                }
            }
            isTestSuitePassed = (isTestPassed && (platformArchive->GetInt32("Success") == 1) );
        }
    }
    
    // create archives if not found
    if(!platformArchive)
    {
        platformArchive = new KeyedArchive();
    }
    
    if(!logArchive) 
    {
        logArchive = new KeyedArchive();
    }
    
    if(!testArchive) 
    {
        testArchive = new KeyedArchive();
    }
    
    if(!testResultsArchive)
    {
        testResultsArchive = new KeyedArchive();
    }
    
    //update test results
    for(int32 i = 0; i < testResults.size(); ++i)
    {
        testResultsArchive->SetInt32(testResults[i].first, (int32)testResults[i].second);
    }
  
    //update test object
    testArchive->SetInt32("RunId", testsId);
    testArchive->SetInt32("Success", (int32)isTestPassed);
    testArchive->SetString("File", testFileName);
    testArchive->SetArchive(testResultsKey, testResultsArchive);

    //update log object
    logArchive->SetArchive(testName, testArchive);
   
    //update platform object
    platformArchive->SetInt32("RunId", testsId);
    platformArchive->SetInt32("TestsCount", (testIndex + 1));
    platformArchive->SetInt32("Success", (int32)isTestSuitePassed);
    platformArchive->SetInt32(testAndFileName, (int32)isTestPassed);
    platformArchive->SetArchive(logKey, logArchive);
 
    //update DB object
    dbUpdateData->SetInt32("RunId", testsId);
    dbUpdateData->SetArchive(AUTOTESTING_PLATFORM_NAME, platformArchive);

    dbUpdateObject->SaveToDB(dbClient);
    
    // delete created archives
    SafeRelease(platformArchive);
    SafeRelease(logArchive);
    SafeRelease(testArchive);
    SafeRelease(testResultsArchive);
    
    // delete created update object
    SafeRelease(dbUpdateObject);
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
            OnError(Format("parsing %s failed - no root node", yamlFilePath.c_str()));
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
                Logger::Debug("AddActionsFromYamlNode action=%s", actionName.c_str());
                if(actionName == "ExecuteYaml")
                {
                    YamlNode* pathNode = actionNode->Get("path");
                    if(pathNode)
                    {
                        AddActionsFromYaml(pathNode->AsString());
                    }
                    else
                    {
                        OnError(Format("AddActionsFromYamlNode action %s no path", actionName.c_str()));
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
                            OnError(Format("AddActionsFromYamlNode action %s no path", actionName.c_str()));
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
                            OnError(Format("AddActionsFromYamlNode action %s no path", actionName.c_str()));
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
                        OnError(Format("AddActionsFromYamlNode action %s no point", actionName.c_str()));
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
                        OnError(Format("AddActionsFromYamlNode action %s no path", actionName.c_str()));
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
                        Logger::Warning("AddActionsFromYamlNode action %s no time", actionName.c_str());
                    }
                }
                else if(actionName == "WaitForUI")
                {
                    float32 timeout = 10.0f;
                    YamlNode* timeoutNode = actionNode->Get("timeout");
                    if(timeoutNode)
                    {
                        timeout = timeoutNode->AsFloat();
                    }

                    YamlNode* controlPathNode = actionNode->Get("controlPath");
                    if(controlPathNode)
                    {
                        WaitForUI(ParseControlPath(controlPathNode), timeout);
                    }
                    else
                    {
                        OnError(Format("AddActionsFromYamlNode action %s no path", actionName.c_str()));
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
                        OnError(Format("AddActionsFromYamlNode action %s no key", actionName.c_str()));
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
                        OnError(Format("AddActionsFromYamlNode action %s no text", actionName.c_str()));
                    }
                }
                else if(actionName == "Assert")
                {
                    YamlNode* messageNode = actionNode->Get("message");
                    String messageText = "";
                    if(messageNode)
                    {
                        messageText = messageNode->AsString();
                    }

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
                                    AssertText(ParseControlPath(expectedControlPathNode), ParseControlPath(actualControlPathNode), messageText);
                                }
                                else
                                {
                                    //TODO: other supported actual getters for expected getter "GetText"
                                    OnError(Format("AddActionsFromYamlNode action %s wrong actual %s for expected %s", actionName.c_str(), actualGetterName.c_str(), expectedGetterName.c_str()));
                                }
                            }
                            else if(expectedGetterName == "FindControl")
                            {
                                if(actualGetterName == "FindControl")
                                {
                                    AssertBool(ParseControlPath(expectedControlPathNode), ParseControlPath(actualControlPathNode), messageText);
                                }
                                else
                                {
                                    //TODO: other supported actual getters for expected getter "FindControl"
                                    OnError(Format("AddActionsFromYamlNode action %s wrong actual %s for expected %s", actionName.c_str(), actualGetterName.c_str(), expectedGetterName.c_str()));
                                }
                            }
                        }
                        else if(actualGetterNode)
                        {
                            String actualGetterName = actualGetterNode->AsString();
                            YamlNode* actualControlPathNode = actualNode->Get("controlPath");
                            if(actualGetterName == "GetText")
                            {
                                AssertText(expectedNode->AsWString(), ParseControlPath(actualControlPathNode), messageText);
                            }
                            else if(actualGetterName == "FindControl")
                            {
                                AssertBool(expectedNode->AsBool(), ParseControlPath(actualControlPathNode), messageText);
                            }
                        }
                        else
                        {
                            OnError(Format("AddActionsFromYamlNode action %s no actual getter", actionName.c_str()));
                        }
                    }
                    else
                    {
                        OnError(Format("AddActionsFromYamlNode action %s no expected or actual", actionName.c_str()));
                    }
                }
                else if(actionName == "Scroll")
                {
                    float32 timeout = 10.0f;
                    YamlNode* timeoutNode = actionNode->Get("timeout");
                    if(timeoutNode)
                    {
                        timeout = timeoutNode->AsFloat();
                    }

                    int32 id = 1;
                    YamlNode* idNode = actionNode->Get("id");
                    if(idNode)
                    {
                        id = idNode->AsInt();
                    }

                    YamlNode* controlPathNode = actionNode->Get("controlPath");
                    if(controlPathNode)
                    {
                        Scroll(ParseControlPath(controlPathNode), id, timeout);
                    }
                    else
                    {
                        OnError(Format("AddActionsFromYamlNode action %s no path", actionName.c_str()));
                    }
                }
                else
                {
                    //TODO: other actions, asserts, getters

                    OnError(Format("AddActionsFromYamlNode wrong action %s", actionName.c_str()));
                }
            }
            else
            {
                OnError("AddActionsFromYamlNode no action");
            }
        }
    }
    else
    {
        OnError("AddActionsFromYamlNode no actions");
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
                    OnError("ParseControlPath part failed");
                }
            }
        }
    }
    else
    {
        OnError("ParseControlPath failed");
    }
    return controlPath;
}

void AutotestingSystem::RunTests()
{
    if(!isInit) return;
    
    if(!isRunning)
    {
        OnTestsSatrted();
        
        isRunning = true;
    }
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
                if(currentAction)
                {
                    currentAction->Execute();
                }
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
    
void AutotestingSystem::OnTestsSatrted()
{
    Logger::Debug("AutotestingSystem::OnTestsStarted");
    AddTestResult("started", true);
}
    
void AutotestingSystem::OnTestAssert(const String & text, bool isPassed)
{
    String assertMsg = Format("%s: %s %s", testName.c_str(), text.c_str(), (isPassed ? "PASSED" : "FAILED"));
    Logger::Debug("AutotestingSystem::OnTestAssert %s", assertMsg.c_str());
    
    AddTestResult(text, isPassed);
    
#ifdef __DAVAENGINE_AUTOTESTING_FILE__
    if(reportFile)
    {
        reportFile->WriteLine(assertMsg);
    }
#endif
}

void AutotestingSystem::OnError(const String & errorMessage)
{
    Logger::Error("AutotestingSystem::OnError %s",errorMessage.c_str());
    
    AddTestResult(errorMessage, false);
    SaveTestToDB();
    
#ifdef __DAVAENGINE_AUTOTESTING_FILE__
    String exitOnErrorMsg = Format("EXIT %s OnError %s", testName.c_str(), errorMessage.c_str());
    if(reportFile)
    {
        reportFile->WriteLine(exitOnErrorMsg);
    }
    SafeRelease(reportFile);
#endif
    ExitApp();
}    
    
void AutotestingSystem::OnTestsFinished()
{
    Logger::Debug("AutotestingSystem::OnTestsFinished");
    
    AddTestResult("finished", true);
    SaveTestToDB();
    
#ifdef __DAVAENGINE_AUTOTESTING_FILE__
    if(reportFile)
    {
        reportFile->WriteLine(Format("EXIT %s OnTestsFinished", testName.c_str()));
    }
    SafeRelease(reportFile);
#endif
    ExitApp();
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

void AutotestingSystem::WaitForUI(const String &controlName, float32 timeout)
{
    WaitForUIAction* waitForUIAction = new WaitForUIAction(controlName, timeout);
    AddAction(waitForUIAction);
    SafeRelease(waitForUIAction);
}

void AutotestingSystem::WaitForUI(const Vector<String> &controlPath, float32 timeout)
{
    WaitForUIAction* waitForUIAction = new WaitForUIAction(controlPath, timeout);
    AddAction(waitForUIAction);
    SafeRelease(waitForUIAction);
}

void AutotestingSystem::Scroll(const String &controlName, int32 id, float32 timeout)
{
    ScrollControlAction* scrollControlAction = new ScrollControlAction(controlName, id, timeout);
    AddAction(scrollControlAction);
    SafeRelease(scrollControlAction);
}

void AutotestingSystem::Scroll(const Vector<String> &controlPath, int32 id, float32 timeout)
{
    ScrollControlAction* scrollControlAction = new ScrollControlAction(controlPath, id, timeout);
    AddAction(scrollControlAction);
    SafeRelease(scrollControlAction);
}

void AutotestingSystem::AssertText(const WideString &expected, const Vector<String> &controlPath, const String &assertMessage)
{
    AssertAction* assertTextAction = new AssertAction(assertMessage);
    
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

void AutotestingSystem::AssertText(const Vector<String> &expectedControlPath, const Vector<String> &actualControlPath, const String &assertMessage)
{
    AssertAction* assertTextAction = new AssertAction(assertMessage);
    
    ControlTextGetter* expectedGetter = new ControlTextGetter(expectedControlPath);
    assertTextAction->SetActualGetter(expectedGetter);
    SafeRelease(expectedGetter);

    ControlTextGetter* actualGetter = new ControlTextGetter(actualControlPath);
    assertTextAction->SetActualGetter(actualGetter);
    SafeRelease(actualGetter);

    AddAction(assertTextAction);
    SafeRelease(assertTextAction);
}
    
void AutotestingSystem::AssertBool(bool expected, const Vector<String> &controlPath, const String &assertMessage)
{
    AssertAction* assertBoolAction = new AssertAction(assertMessage);
    
    VariantType expectedValue;
    expectedValue.SetBool(expected);
    Getter* expectedGetter = new Getter(expectedValue);
    assertBoolAction->SetExpectedGetter(expectedGetter);
    SafeRelease(expectedGetter);

    ControlBoolGetter* actualGetter = new ControlBoolGetter(controlPath);
    assertBoolAction->SetActualGetter(actualGetter);
    SafeRelease(actualGetter);

    AddAction(assertBoolAction);
    SafeRelease(assertBoolAction);
}

void AutotestingSystem::AssertBool(const Vector<String> &expectedControlPath, const Vector<String> &actualControlPath, const String &assertMessage)   
{
    AssertAction* assertBoolAction = new AssertAction(assertMessage);
    
    ControlBoolGetter* expectedGetter = new ControlBoolGetter(expectedControlPath);
    assertBoolAction->SetActualGetter(expectedGetter);
    SafeRelease(expectedGetter);

    ControlBoolGetter* actualGetter = new ControlBoolGetter(actualControlPath);
    assertBoolAction->SetActualGetter(actualGetter);
    SafeRelease(actualGetter);

    AddAction(assertBoolAction);
    SafeRelease(assertBoolAction);
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
#ifndef __DAVAENGINE_IPHONE__
    case UIEvent::PHASE_MOVE:
        {
            mouseMove = input;
            if(IsTouchDown(id))
            {
                Logger::Error("AutotestingSystem::OnInput PHASE_MOVE id=%d must be PHASE_DRAG",id);
            }
        }
        break;
#endif
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

void AutotestingSystem::ExitApp()
{
    //TODO: exit app on each platform
    Core::Instance()->Quit();
}

};

#endif //__DAVAENGINE_AUTOTESTING__