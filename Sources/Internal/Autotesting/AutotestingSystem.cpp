#include "Autotesting/AutotestingSystem.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Core/Core.h"
#include "Render/RenderHelper.h"
#include "FileSystem/FileList.h"

#ifdef AUTOTESTING_LUA
#include "AutotestingSystemLua.h"
#else
#include "AutotestingSystemYaml.h"
#endif

namespace DAVA
{

AutotestingSystem::AutotestingSystem()
    : isInit(false)
    , isRunning(false)
    , needExitApp(false)
    , timeBeforeExit(0.0f)
    , testsId(0)
    , testsDate(0)
    , testIndex(0)
    , dbClient(NULL)
    , isDB(false)
    , needClearDB(false)
    , reportFile(NULL)
{
#ifdef AUTOTESTING_LUA
    new AutotestingSystemLua();
#else
    new AutotestingSystemYaml();
#endif
}

AutotestingSystem::~AutotestingSystem()
{
#ifdef AUTOTESTING_LUA
    AutotestingSystemLua::Instance()->Release();
#else
    AutotestingSystemYaml::Instance()->Release();
#endif
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
        
        uint32 autotestingId = 1;
        
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
            isDB = true;
        }
		else
		{
			// build for local testing
			isDB = false;
		}
        SafeRelease(file);
        
		if(isDB)
		{
			if(!ConnectToDB())
			{
				return;
			}
		}
        
        // compare ids
        if(testsId != autotestingId)
        {
            testIndex = 0;
            testsId = autotestingId;
			needClearDB = true;
        }
        
        int32 indexInFileList = testIndex;
        int32 fileCount = fileList.GetFileCount();
        // skip directories
        for(int32 i = 0; (i <= indexInFileList) && (i < fileListSize); ++i)
        {
            if((fileList.IsDirectory(i)) || (fileList.IsNavigationDirectory(i)))
            {
                indexInFileList++;
            }
            else
            {
                String fileExtension = FileSystem::Instance()->GetExtension(fileList.GetFilename(i));
#ifdef AUTOTESTING_LUA
                //skip all non-lua files
                if(fileExtension != ".lua")
                {
                    indexInFileList++;
                }
#else
                // skip all non-yaml files
                if(fileExtension != ".yaml")
                {
                    indexInFileList++;
                }
#endif
            }
        }
        
        if(indexInFileList < fileListSize)
        {
            testFilePath = fileList.GetPathname(indexInFileList);
            testFileName = fileList.GetFilename(indexInFileList);
            
            
#ifdef AUTOTESTING_LUA
            AutotestingSystemLua::Instance()->InitFromFile(testFilePath);
#else
            AutotestingSystemYaml::Instance()->InitFromYaml(testFilePath);
#endif
            
            // save next index and autotesting id
            autotestingArchive->SetUInt32("id", testsId);
            autotestingArchive->SetInt32("index", (testIndex + 1));
        }
        else
        {
            // last file (or after last) - reset id and index
            //autotestingArchive->SetUInt32("id", 0); //don't reset id - allow cycled tests
            autotestingArchive->SetInt32("index", 0);
        }

//        if((fileCount - 1) <= testIndex)
//        {
//            // last file - reset id and index
//            //autotestingArchive->SetUInt32("id", 0); //don't reset id - allow cycled tests
//            autotestingArchive->SetInt32("index", 0);
//        }
//        else
//        {
//            // save next index and autotesting id
//            autotestingArchive->SetUInt32("id", testsId);
//            autotestingArchive->SetInt32("index", (testIndex + 1));
//        }
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
    
void AutotestingSystem::RunTests()
{
    if(!isInit) return;
    
    if(!isRunning)
    {
        OnTestsSatrted();
        
        isRunning = true;
    }
}
    
void AutotestingSystem::Init(const String &_testName)
{
    if(!isInit)
    {
        testReportsFolder = "~doc:/autotesting";
        FileSystem::Instance()->CreateDirectory(FileSystem::Instance()->SystemPathForFrameworkPath(testReportsFolder), true);
        reportFile = DAVA::File::Create(Format("%s/autotesting.report",testReportsFolder.c_str()), DAVA::File::CREATE|DAVA::File::WRITE);
        
        isInit = true;
        testName = _testName;
    }
}
    
void AutotestingSystem::SetProjectName(const String &_projectName)
{
    projectName = _projectName;
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
    if(!isDB) return;
    
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
    
    int32 testResultsCount = testResults.size();
    bool isTestPassed = true; // if only started should not count as success
    bool isStartLogged = false;
    bool isFinishLogged = false;
    for(int32 i = 0; i < testResultsCount; ++i)
    {
        if(!testResults[i].second)
        {
            isTestPassed = false;
            break;
        }
        
        if(testResults[i].first == "started")
        {
            isStartLogged = true;
        }
        else if(testResults[i].first == "finished")
        {
            isFinishLogged = true;
        }
    }
    
    if(isTestPassed)
    {
        isTestPassed = (isStartLogged && isFinishLogged);
    }
    
    bool isTestSuitePassed = isTestPassed;
    
    // find platform object
    if(isFound)
    {
        //found database object
        
        if(needClearDB)
        {
            needClearDB = false;
        }
        else
        {
            // find platform object
            platformArchive = SafeRetain(dbUpdateData->GetArchive(AUTOTESTING_PLATFORM_NAME, NULL));
        }
        
        if(platformArchive)
        {
            // found platform object
            // find log object
            logArchive = SafeRetain(platformArchive->GetArchive(logKey, NULL));
            
            if(logArchive)
            {
                // found log object
                
                //find all test objects to set platform test results (if all tests passed for current platform)
                if(isTestSuitePassed)
                {
                    const Map<String, VariantType*> &logArchiveData = logArchive->GetArchieveData();
                    for(Map<String, VariantType*>::const_iterator it = logArchiveData.begin(); it != logArchiveData.end(); ++it)
                    {
                        if((it->first != "_id") && it->second)
                        {
                            KeyedArchive *tmpTestArchive = it->second->AsKeyedArchive();
                            if(tmpTestArchive)
                            {
                                isTestSuitePassed &= (tmpTestArchive->GetInt32("Success") == 1);
                            }
                        }
                    }
                }
                
                // find test object
                testArchive = SafeRetain(logArchive->GetArchive(testAndFileName, NULL));
                if(testArchive)
                {
                    // found test object
                    // find test results
                    testResultsArchive = SafeRetain(testArchive->GetArchive(testResultsKey));
                }
            }
            isTestSuitePassed &= isTestPassed;
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
    for(int32 i = 0; i < testResultsCount; ++i)
    {
        bool testResultSuccess = testResults[i].second;
        testResultsArchive->SetInt32(testResults[i].first, (int32)testResultSuccess);
    }
    
    //update test object
    testArchive->SetInt32("RunId", testsId);
    testArchive->SetInt32("Success", (int32)isTestPassed);
    testArchive->SetString("File", testFileName);
    testArchive->SetString("Name", testName);
    testArchive->SetArchive(testResultsKey, testResultsArchive);
    
    //update log object
    logArchive->SetArchive(testAndFileName, testArchive);
    
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

void AutotestingSystem::Update(float32 timeElapsed)
{
    if(!isInit) return;
    
    if(needExitApp)
    {
        timeBeforeExit -= timeElapsed;
        if(timeBeforeExit <= 0.0f)
        {
            needExitApp = false;
            Core::Instance()->Quit();
        }
        return;
    }

    if(isRunning)
    {
#ifdef AUTOTESTING_LUA
        AutotestingSystemLua::Instance()->Update(timeElapsed);
#else
        AutotestingSystemYaml::Instance()->Update(timeElapsed);
#endif
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
	SaveTestToDB();
#ifdef AUTOTESTING_LUA
    AutotestingSystemLua::Instance()->StartTest();
#endif
}
    
void AutotestingSystem::OnTestAssert(const String & text, bool isPassed)
{
    String assertMsg = Format("%s: %s %s", testName.c_str(), text.c_str(), (isPassed ? "PASSED" : "FAILED"));
    Logger::Debug("AutotestingSystem::OnTestAssert %s", assertMsg.c_str());
    
    AddTestResult(text, isPassed);
	SaveTestToDB();
    
	if(reportFile)
	{
		reportFile->WriteLine(assertMsg);
	}

	if(!isPassed)
	{
		SafeRelease(reportFile);
		ExitApp();
	}
}

void AutotestingSystem::OnMessage(const String & logMessage)
{
	Logger::Debug("AutotestingSystem::OnMessage %s", logMessage.c_str());
    
	String logMsg = Format("%s OnMessage %s", testName.c_str(), logMessage.c_str());
	if(reportFile)
	{
		reportFile->WriteLine(logMsg);
	}
}

void AutotestingSystem::OnError(const String & errorMessage)
{
    Logger::Error("AutotestingSystem::OnError %s",errorMessage.c_str());
    
    AddTestResult(errorMessage, false);
	SaveTestToDB();
	
	String exitOnErrorMsg = Format("EXIT %s OnError %s", testName.c_str(), errorMessage.c_str());
	if(reportFile)
	{
		reportFile->WriteLine(exitOnErrorMsg);
	}
	SafeRelease(reportFile);

    ExitApp();
}    
    
void AutotestingSystem::OnTestsFinished()
{
    Logger::Debug("AutotestingSystem::OnTestsFinished");
    
    isRunning = false;
    
    AddTestResult("finished", true);
	SaveTestToDB();
    
    if(reportFile)
    {
        reportFile->WriteLine(Format("EXIT %s OnTestsFinished", testName.c_str()));
    }
    SafeRelease(reportFile);

    ExitApp();
}

void AutotestingSystem::OnInput(const UIEvent &input)
{
    Logger::Debug("AutotestingSystem::OnInput %d phase=%d count=%d point=(%f, %f) physPoint=(%f,%f) key=%c",input.tid, input.phase, input.tapCount, input.point.x, input.point.y, input.physPoint.x, input.physPoint.y, input.keyChar);
    
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
                Logger::Error("AutotestingSystemYaml::OnInput PHASE_BEGAN duplicate touch id=%d",id);
            }
        }
            break;
#ifndef __DAVAENGINE_IPHONE__
        case UIEvent::PHASE_MOVE:
        {
            mouseMove = input;
            if(IsTouchDown(id))
            {
                Logger::Error("AutotestingSystemYaml::OnInput PHASE_MOVE id=%d must be PHASE_DRAG",id);
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
                Logger::Error("AutotestingSystemYaml::OnInput PHASE_DRAG id=%d must be PHASE_MOVE",id);
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
                Logger::Error("AutotestingSystemYaml::OnInput PHASE_ENDED id=%d not found",id);
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
    if(!needExitApp)
    {
        needExitApp = true;
        timeBeforeExit = 1.0f;
    }
}

};

#endif //__DAVAENGINE_AUTOTESTING__