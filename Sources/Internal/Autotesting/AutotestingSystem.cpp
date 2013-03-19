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
    , groupName("default")
    , isMaster(true)
    , requestedHelpers(0)
    , isWaiting(false)
    , isRegistered(false)
    , waitTimeLeft(0.0f)
    , waitCheckTimeLeft(0.0f)
    , masterRunId(0)
    , isInitMultiplayer(false)
    , stepIndex(0)
    , logIndex(0)
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
            stepIndex = 0;
            logIndex = 0;
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
            
            if(!file->IsEof())
            {
                char groupCharName[128];
                file->ReadLine(tempBuf, 1024);
                sscanf(tempBuf, "%s", groupCharName);
                String gName = groupCharName;
                if(!gName.empty())
                {
                    groupName = gName;
                }
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
            stepIndex = 0;
            logIndex = 0;
            testsId = autotestingId;
			needClearDB = true;
        }
        
        int32 indexInFileList = GetIndexInFileList(fileList, testIndex);
        
        // try to cycle
        if(fileListSize <= indexInFileList)
        {
            testIndex = 0;
            stepIndex = 0;
            logIndex = 0;
            indexInFileList = GetIndexInFileList(fileList, testIndex);
        }
        
        if(indexInFileList < fileListSize)
        {
            // found direct or cycled
            
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
            // not found
            
            // last file (or after last) - reset id and index
            //autotestingArchive->SetUInt32("id", 0); //don't reset id - allow cycled tests
            autotestingArchive->SetInt32("index", 0);
        }
        autotestingArchive->Save("~doc:/autotesting/autotesting.archive");
    }
}
    
    
int32 AutotestingSystem::GetIndexInFileList(FileList &fileList, int32 index)
{
    int32 fileListSize = fileList.GetCount();
    int32 indexInFileList = index;
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
    return indexInFileList;
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
        isRunning = true;
        OnTestsSatrted();
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
    
//void AutotestingSystem::AddTestResult(const String &text, bool isPassed, const String &error)
//{
//    testResults.push_back(TestResult(text, isPassed, error));
//}
    
#define AUTOTESTING_TESTS "Tests"
#define AUTOTESTING_STEPS "Steps"
#define AUTOTESTING_LOG "Log"
    
KeyedArchive *AutotestingSystem::FindOrInsertTestArchive(MongodbUpdateObject* dbUpdateObject, const String &testId)
{
    Logger::Debug("AutotestingSystem::FindOrInsertTestArchive testId=%s", testId.c_str());
    KeyedArchive* currentTestArchive = NULL;
    
    KeyedArchive* platformArchive = NULL;
    KeyedArchive* groupArchive = NULL;
    KeyedArchive* testsArchive = NULL;
    
    String testsName = Format("%u",testsDate);
    Logger::Debug("AutotestingSystem::FindOrInsertTestArchive testsName=%s", testsName.c_str());
    
    bool isFound = dbClient->FindObjectByKey(testsName, dbUpdateObject);
    if(!isFound)
    {
        dbUpdateObject->SetObjectName(testsName);
        Logger::Debug("AutotestingSystem::FindOrInsertTestArchive new MongodbUpdateObject");
    }
    dbUpdateObject->LoadData();
    
    KeyedArchive* dbUpdateData = dbUpdateObject->GetData();
    
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
            // find group object
            groupArchive = SafeRetain(platformArchive->GetArchive(groupName, NULL));
            
            if(groupArchive)
            {
//                if(testIndex == 0 && isMaster)
//                {
//                    // remove prev results
//                    // only for master tests, helpers should just overwrite results (assumed they never share the same group with master)
//                    SafeRelease(groupArchive);
//                }
//                else
//                {
                    // found group object
                    // find tests object
                    testsArchive = SafeRetain(groupArchive->GetArchive(AUTOTESTING_TESTS, NULL));
                    
                    if(testsArchive)
                    {
                        // find test object
                        currentTestArchive = SafeRetain(testsArchive->GetArchive(testId, NULL));
                    }
//                }
            }
        }
    }
    
    if(!currentTestArchive)
    {
        currentTestArchive = new KeyedArchive();
        
        currentTestArchive->SetString("Name", testName);
        currentTestArchive->SetString("FileName", testFileName);
        
        if(!testsArchive)
        {
            testsArchive = new KeyedArchive();
            if(!groupArchive)
            {
                groupArchive = new KeyedArchive();
                if(!platformArchive)
                {
                    platformArchive = new KeyedArchive();
                    dbUpdateData->SetArchive(AUTOTESTING_PLATFORM_NAME, platformArchive);
                    Logger::Debug("AutotestingSystem::FindOrInsertTestArchive new %s", AUTOTESTING_PLATFORM_NAME);
                    SafeRelease(platformArchive);
                    platformArchive = SafeRetain(dbUpdateData->GetArchive(AUTOTESTING_PLATFORM_NAME));
                }
                platformArchive->SetArchive(groupName, groupArchive);
                Logger::Debug("AutotestingSystem::FindOrInsertTestArchive new %s", groupName.c_str());
                SafeRelease(groupArchive);
                groupArchive = SafeRetain(platformArchive->GetArchive(groupName));
            }
            groupArchive->SetArchive(AUTOTESTING_TESTS, testsArchive);
            Logger::Debug("AutotestingSystem::FindOrInsertTestArchive new %s", AUTOTESTING_TESTS);
            SafeRelease(testsArchive);
            testsArchive = SafeRetain(groupArchive->GetArchive(AUTOTESTING_TESTS));
        }
        
        testsArchive->SetArchive(testId, currentTestArchive);
        Logger::Debug("AutotestingSystem::FindOrInsertTestArchive new %s", testId.c_str());
    }
    SafeRelease(currentTestArchive);
    currentTestArchive = testsArchive->GetArchive(testId);
    SafeRelease(testsArchive);
    SafeRelease(groupArchive);
    SafeRelease(platformArchive);
    
    return currentTestArchive;
}
    
KeyedArchive *AutotestingSystem::FindOrInsertTestStepArchive(KeyedArchive *testArchive, const String &stepId)
{
    Logger::Debug("AutotestingSystem::FindOrInsertTestStepArchive stepId=%s", stepId.c_str());
    
    KeyedArchive* currentTestStepArchive = NULL;
    KeyedArchive* testStepsArchive = NULL;
    
    testStepsArchive = SafeRetain(testArchive->GetArchive(AUTOTESTING_STEPS, NULL));
    if(testStepsArchive)
    {
        currentTestStepArchive = SafeRetain(testStepsArchive->GetArchive(stepId, NULL));
    }
    
    if(!currentTestStepArchive)
    {
        currentTestStepArchive = new KeyedArchive();
        if(!testStepsArchive)
        {
            testStepsArchive = new KeyedArchive();
            testArchive->SetArchive(AUTOTESTING_STEPS, testStepsArchive);
            SafeRelease(testStepsArchive);
            testStepsArchive = SafeRetain(testArchive->GetArchive(AUTOTESTING_STEPS));
        }
        testStepsArchive->SetArchive(stepId, currentTestStepArchive);
        Logger::Debug("AutotestingSystem::FindOrInsertTestStepArchive new %s", stepId.c_str());
    }
    SafeRelease(currentTestStepArchive);
    currentTestStepArchive = testStepsArchive->GetArchive(stepId);
    SafeRelease(testStepsArchive);
    
    return currentTestStepArchive;
}
    
KeyedArchive *AutotestingSystem::FindOrInsertTestStepLogEntryArchive(KeyedArchive *testStepArchive, const String &logId)
{
    Logger::Debug("AutotestingSystem::FindOrInsertTestStepLogEntryArchive logId=%s", logId.c_str());
    
    KeyedArchive* currentTestStepLogEntryArchive = NULL;
    KeyedArchive* testStepLogArchive = NULL;
    
    testStepLogArchive = SafeRetain(testStepArchive->GetArchive(AUTOTESTING_LOG, NULL));
    if(testStepLogArchive)
    {
        currentTestStepLogEntryArchive = SafeRetain(testStepLogArchive->GetArchive(logId, NULL));
    }
    
    if(!currentTestStepLogEntryArchive)
    {
        currentTestStepLogEntryArchive = new KeyedArchive();
        if(!testStepLogArchive)
        {
            testStepLogArchive = new KeyedArchive();
            testStepArchive->SetArchive(AUTOTESTING_LOG, testStepLogArchive);
            SafeRelease(testStepLogArchive);
            testStepLogArchive = SafeRetain(testStepArchive->GetArchive(AUTOTESTING_LOG));
        }
        testStepLogArchive->SetArchive(logId, currentTestStepLogEntryArchive);
    }
    SafeRelease(currentTestStepLogEntryArchive);
    currentTestStepLogEntryArchive = testStepLogArchive->GetArchive(logId);
    SafeRelease(testStepLogArchive);
    
    return currentTestStepLogEntryArchive;
}

String AutotestingSystem::GetCurrentTimeString()
{
    uint64 timeAbsMs = SystemTimer::Instance()->FrameStampTimeMS();
    uint16 hours = (timeAbsMs/3600000)%60;
    uint16 minutes = (timeAbsMs/60000)%60;
    uint16 seconds = (timeAbsMs/1000)%60;
    return Format("%02d:%02d:%02d", hours, minutes, seconds);
}
    
void AutotestingSystem::SaveTestStepToDB(const String &stepDescription, bool isPassed, const String &error)
{
    if(!isDB) return;
    
    Logger::Debug("AutotestingSystem::SaveTestStepToDB %s %d %s", stepDescription.c_str(), isPassed, error.c_str());
    
    String testId = Format("Test%d", testIndex);
    String stepId = Format("Step%d", ++stepIndex);
    
    MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
    KeyedArchive* currentTestArchive = FindOrInsertTestArchive(dbUpdateObject, testId);
    KeyedArchive* currentTestStepArchive = FindOrInsertTestStepArchive(currentTestArchive, stepId);
    
    currentTestStepArchive->SetBool("Success", isPassed);
    currentTestStepArchive->SetString("Description", stepDescription);
    
    Logger::Debug("AutotestingSystem::SaveTestStepToDB testId=%s stepId=%s", testId.c_str(), stepId.c_str());
    
    if(!error.empty())
    {
        String logId = Format("Message%d", ++logIndex);
        KeyedArchive* currentLogEntryArchive = FindOrInsertTestStepLogEntryArchive(currentTestStepArchive, logId);
        
        currentLogEntryArchive->SetString("Type", "ERROR");
        String currentTime = GetCurrentTimeString();
        Logger::Debug("currentTime=%s", currentTime.c_str());
        currentLogEntryArchive->SetString("Time", currentTime);
        currentLogEntryArchive->SetString("Message", error);
    }
    logIndex = 0;
    
    dbUpdateObject->SaveToDB(dbClient);
    SafeRelease(dbUpdateObject);
}
    
void AutotestingSystem::SaveTestStepLogEntryToDB(const String &type, const String &time, const String &message)
{
    if(!isDB) return;
    
    Logger::Debug("AutotestingSystem::SaveTestStepLogEntryToDB %s %s %s", type.c_str(), time.c_str(), message.c_str());
    
    String testId = Format("Test%d", testIndex);
    String stepId = Format("Step%d", stepIndex);
    String logId = Format("Message%d", ++logIndex);

    String testsName = Format("%u",testsDate);
    
    MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
    KeyedArchive* currentTestArchive = FindOrInsertTestArchive(dbUpdateObject, testId);
    KeyedArchive* currentTestStepArchive = FindOrInsertTestStepArchive(currentTestArchive, stepId);
    KeyedArchive* currentLogEntryArchive = FindOrInsertTestStepLogEntryArchive(currentTestStepArchive, logId);

    currentLogEntryArchive->SetString("Type", type);
    currentLogEntryArchive->SetString("Time", time);
    currentLogEntryArchive->SetString("Message", message);
    
    dbUpdateObject->SaveToDB(dbClient);
    
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
    else if(isWaiting)
    {
        waitTimeLeft -= timeElapsed;
        waitCheckTimeLeft -= timeElapsed;
        
        if(waitTimeLeft <= 0.0f)
        {
            isWaiting = false;
            isRunning = false;
            OnError("Multiplayer Wait Timeout");
        }
        else if(waitCheckTimeLeft <= 0.0f)
        {
            waitCheckTimeLeft = 0.1f;
            if(CheckMasterHelpersReadyDB())
            {
                isWaiting = false;
                isRunning = true;
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
#ifdef AUTOTESTING_LUA
    AutotestingSystemLua::Instance()->StartTest();
#endif
}
    
#define AUTOTEST_MASTER_ID "master"
String AutotestingSystem::ReadMasterIDFromDB()
{
    //TODO: get first available master
    return AUTOTEST_MASTER_ID;
}
    
void AutotestingSystem::InitMultiplayer(bool _isMaster)
{
    if(!isInitMultiplayer)
    {
        isInitMultiplayer = true;
 
        isMaster = _isMaster;
        masterId = ReadMasterIDFromDB(); //TODO: DB set or get name
        
        multiplayerName = Format("%u_multiplayer", testsDate);
        Logger::Debug("AutotestingSystem::InitMultiplayer %s", multiplayerName.c_str());
        isRunning = false;
        isWaiting = true;
        waitTimeLeft = 300.0f;
    }
}
    
void AutotestingSystem::RegisterMasterInDB(int32 helpersCount)
{
    requestedHelpers = helpersCount;
    
    MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
    if(!dbClient->FindObjectByKey(multiplayerName, dbUpdateObject))
    {
        dbUpdateObject->SetObjectName(multiplayerName);
    }
    dbUpdateObject->LoadData();
    
    KeyedArchive* masterArchive = SafeRetain(dbUpdateObject->GetData()->GetArchive(masterId, NULL));
    if(!masterArchive)
    {
        masterArchive = new KeyedArchive();
    }
    
    masterArchive->SetInt32("requested", requestedHelpers);
    masterArchive->SetInt32("helpers", 0);
    
    masterRunId = masterArchive->GetInt32("runId", 0) + 1;
    masterArchive->SetInt32("runId", masterRunId);
    
    masterArchive->SetInt32("run", 0);
    //TODO: set task for helpers into DB
    masterArchive->SetString("task", testFileName);
    
    dbUpdateObject->GetData()->SetArchive(masterId, masterArchive);
    
    isRegistered = dbUpdateObject->SaveToDB(dbClient);
    Logger::Debug("AutotestingSystem::RegisterMasterInDB %d", isRegistered);
    
    // delete created archives
    SafeRelease(masterArchive);
    
    // delete created update object
    SafeRelease(dbUpdateObject);
}
    
void AutotestingSystem::RegisterHelperInDB()
{
    MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
    if(dbClient->FindObjectByKey(multiplayerName, dbUpdateObject))
    {
        dbUpdateObject->LoadData();
        
        KeyedArchive* masterArchive = SafeRetain(dbUpdateObject->GetData()->GetArchive(masterId, NULL));
        if(masterArchive)
        {
            if(masterArchive->GetInt32("run", 0) == 0)
            {
                int32 helpersCount = masterArchive->GetInt32("helpers", 0) + 1;
                masterArchive->SetInt32("helpers", helpersCount);
                
                masterRunId = masterArchive->GetInt32("runId", 0);
                
                dbUpdateObject->GetData()->SetArchive(masterId, masterArchive);
                
                isRegistered = dbUpdateObject->SaveToDB(dbClient);
                Logger::Debug("AutotestingSystem::RegisterHelperInDB %d", isRegistered);
            }
        }
        // delete created archives
        SafeRelease(masterArchive);
    }
    // delete created update object
    SafeRelease(dbUpdateObject);
}
    
bool AutotestingSystem::CheckMasterHelpersReadyDB()
{
    //Logger::Debug("AutotestingSystem::CheckMasterHelpersReadyDB");
    bool isReady = false;
    
    if(!isRegistered)
    {
        if(isMaster)
        {
            RegisterMasterInDB(requestedHelpers);
        }
        else
        {
            RegisterHelperInDB();
        }
    }
    
    if(isRegistered)
    {
        MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
        if(dbClient->FindObjectByKey(multiplayerName, dbUpdateObject))
        {
            dbUpdateObject->LoadData();
            KeyedArchive* masterArchive = SafeRetain(dbUpdateObject->GetData()->GetArchive(masterId, NULL));
            if(masterArchive)
            {
                // for registered agents, check if runId is overwritten
                if(masterRunId != masterArchive->GetInt32("runId"))
                {
                    // runId was overwritten
                    if(isMaster)
                    {
                        // exit on error (conflict with another master)
                        OnError("Multiplayer Master Conflict");
                    }
                    else
                    {
                        // re-register helper
                        isRegistered = false;
                    }
                }
                else
                {
                    if(isMaster)
                    {
                        int32 requested = masterArchive->GetInt32("requested");
                        int32 helpers = masterArchive->GetInt32("helpers");
                        if(requested == helpers)
                        {
                            if(requested != requestedHelpers)
                            {
                                OnError("Multiplayer Master wrong Helpers count");
                            }
                            else
                            {
                                masterArchive->SetInt32("run", 1);
                                
                                dbUpdateObject->GetData()->SetArchive(masterId, masterArchive);
                                
                                isReady = dbUpdateObject->SaveToDB(dbClient);
                                if(isReady)
                                {
                                    Logger::Debug("AutotestingSystem::CheckMasterHelpersReadyDB Master: %d helpers ready", requestedHelpers);
                                }
                                else
                                {
                                    Logger::Debug("AutotestingSystem::CheckMasterHelpersReadyDB Master: failed to run");
                                }
                            }
                        }
                    }
                    else
                    {
                        
                        if(masterArchive->GetInt32("run", 0) > 0)
                        {
                            isReady = true;
                            //TODO: get task from DB
                            masterTask = masterArchive->GetString("task");
                            Logger::Debug("AutotestingSystem::CheckMasterHelpersReadyDB Helper: run test %s", masterTask.c_str());
                        }
                    }
                }
            }
            SafeRelease(masterArchive);
        }
        SafeRelease(dbUpdateObject);
    }
    return isReady;
}

void AutotestingSystem::OnTestStep(const String & stepName, bool isPassed, const String &error)
{
    String assertMsg = Format("%s: %s %s", testName.c_str(), stepName.c_str(), (isPassed ? "PASSED" : "FAILED"));
    Logger::Debug("AutotestingSystem::OnTestStep %s", assertMsg.c_str());
    
    //AddTestResult(stepName, isPassed, error);
	//SaveTestToDB();
    
    SaveTestStepToDB(stepName, isPassed, error);
    
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
    
    isWaiting = false;
    
    //AddTestResult(errorMessage, false);
	//SaveTestToDB();
	
    SaveTestStepLogEntryToDB("ERROR", GetCurrentTimeString(), errorMessage);
    
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
    
    //AddTestResult("finished", true);
	//SaveTestToDB();
    
    SaveTestStepToDB("finished", true);
    
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
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)
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