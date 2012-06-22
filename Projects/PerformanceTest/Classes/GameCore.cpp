/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "GameCore.h"

#include "Config.h"
#include "Database/MongodbObject.h"

#include "BaseScreen.h"

#include "SpriteTest.h"
#include "CacheTest.h"
#include "LandscapeTest.h"


using namespace DAVA;

GameCore::GameCore()
{
    logFile = NULL;
    
    dbClient = NULL;
    
    currentScreen = NULL;
    
    currentScreenIndex = 0;
    currentTestIndex = 0;
}

GameCore::~GameCore()
{
}

void GameCore::OnAppStarted()
{
	RenderManager::Instance()->SetFPS(60);

    if(ConnectToDB())
    {
        //TODO: test only
//        dbClient->DropCollection();
//        dbClient->DropDatabase();
        //TODO: test only
        
        new SpriteTest("Sprite");
        new CacheTest("Cache Test");
        new LandscapeTest("Landscape Textures Test", LandscapeNode::TILED_MODE_COUNT);
        new LandscapeTest("Landscape Mixed Mode", LandscapeNode::TILED_MODE_MIXED);
        new LandscapeTest("Landscape Tiled Mode", LandscapeNode::TILED_MODE_TILEMASK);
        new LandscapeTest("Landscape Texture Mode", LandscapeNode::TILED_MODE_TEXTURE);
        
#if defined (SINGLE_MODE)
        RunTestByName(SINGLE_TEST_NAME);
#else //#if defined (SINGLE_MODE)
        RunAllTests();
#endif //#if defined (SINGLE_MODE)
        
    }
    else 
    {
        LogMessage(String("Can't connect to DB"));
        Core::Instance()->Quit();
    }
}

void GameCore::RegisterScreen(BaseScreen *screen)
{
    UIScreenManager::Instance()->RegisterScreen(screen->GetScreenId(), screen);
    screens.push_back(screen);
}


bool GameCore::CreateLogFile()
{
    String documentsPath =      String(FileSystem::Instance()->GetUserDocumentsPath()) 
                            +   "PerfomanceTest/";
    
    bool documentsExists = FileSystem::Instance()->IsDirectory(documentsPath);
    if(!documentsExists)
    {
        FileSystem::Instance()->CreateDirectory(documentsPath);
    }
    FileSystem::Instance()->SetCurrentDocumentsDirectory(documentsPath);
    
    
    String folderPath = FileSystem::Instance()->GetCurrentDocumentsDirectory() + "Reports/";
    bool folderExcists = FileSystem::Instance()->IsDirectory(folderPath);
    if(!folderExcists)
    {
        FileSystem::Instance()->CreateDirectory(folderPath);
    }

	time_t logStartTime = time(0);
	logFile = File::Create(Format("~doc:/Reports/%lld.report", logStartTime), File::CREATE | File::WRITE);

    return (NULL != logFile);
}

void GameCore::OnAppFinished()
{
    if(dbClient)
    {
        dbClient->Disconnect();
        SafeRelease(dbClient);
    }
    
    for(int32 i = 0; i < screens.size(); ++i)
    {
        SafeRelease(screens[i]);
    }
    screens.clear();

    SafeRelease(logFile);
}

void GameCore::OnSuspend()
{
}

void GameCore::OnResume()
{
    ApplicationCore::OnResume();
}

void GameCore::OnBackground()
{	
}

void GameCore::BeginFrame()
{
	ApplicationCore::BeginFrame();
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
}

void GameCore::Update(float32 timeElapsed)
{	
    ProcessTests();
	ApplicationCore::Update(timeElapsed);
}

void GameCore::Draw()
{
	ApplicationCore::Draw();
}

bool GameCore::ConnectToDB()
{
    DVASSERT(NULL == dbClient);
    
    dbClient = MongodbClient::Create(DATABASE_IP, DATAPASE_PORT);
    if(dbClient)
    {
        dbClient->SetDatabaseName(DATABASE_NAME);
        dbClient->SetCollectionName(DATABASE_COLLECTION);
    }
    
    return (NULL != dbClient);
}

void GameCore::RunAllTests()
{
    currentTestIndex = 0;
    for(int32 iScr = 0; iScr < screens.size(); ++iScr)
    {
        int32 count = screens[iScr]->GetTestCount();
        if(0 < count)
        {
            currentScreen = screens[iScr];
            currentScreenIndex = iScr;
            break;
        }
    }
    
    if(currentScreen)
    {
        UIScreenManager::Instance()->SetFirst(currentScreen->GetScreenId());
    }
    else 
    {
        logFile->WriteLine(String("There are no tests."));
        Core::Instance()->Quit();
    }
}

void GameCore::RunTestByName(const String &testName)
{
    currentTestIndex = 0;
    for(int32 iScr = 0; iScr < screens.size(); ++iScr)
    {
        if(screens[iScr]->GetName() == testName)
        {
            currentScreen = screens[iScr];
            currentScreenIndex = iScr;
            break;
        }
    }
    
    if(currentScreen)
    {
        UIScreenManager::Instance()->SetFirst(currentScreen->GetScreenId());
    }
    else 
    {
        logFile->WriteLine(Format("Test %s not found", testName.c_str()));
        Core::Instance()->Quit();
    }
}



void GameCore::FinishTests()
{
    FlushTestResults();
    Core::Instance()->Quit();
}

void GameCore::LogMessage(const String &message)
{
    if(!logFile)
    {
        CreateLogFile();
    }
    
    logFile->WriteLine(message);
}

int32 GameCore::TestCount()
{
    int32 count = 0;
    
    for(int32 i = 0; i < screens.size(); ++i)
    {
        count += screens[i]->GetTestCount();
    }
    
    return count;
}

void GameCore::ProcessTests()
{
    if(currentScreen && currentScreen->ReadyForTests())
    {
        bool ret = currentScreen->RunTest(currentTestIndex);
        
        if(ret)
        {
            ++currentTestIndex;
            if(currentScreen->GetTestCount() == currentTestIndex)
            {
#if defined (SINGLE_MODE)
                FinishTests();
#else //#if defined (SINGLE_MODE)
                ++currentScreenIndex;
                if(currentScreenIndex == screens.size())
                {
                    FinishTests();
                }
                else 
                {
                    currentScreen = screens[currentScreenIndex];
                    currentTestIndex = 0;
                    UIScreenManager::Instance()->SetScreen(currentScreen->GetScreenId());
                }
#endif //#if defined (SINGLE_MODE)
            }
        }
    }
}



void GameCore::FlushTestResults()
{
    if(!dbClient) return;

    MongodbObject *oldPlatformObject = dbClient->FindObjectByKey(PLATFORM_NAME);
    MongodbObject *newPlatformObject = dbClient->CreateObject();
    
    if(newPlatformObject)
    {
        newPlatformObject->SetObjectName(PLATFORM_NAME);
        
        time_t logStartTime = time(0);
        String testTimeString = Format("%lld", logStartTime);

        for(int32 iScr = 0; iScr < screens.size(); ++iScr)
        {
            int32 count = screens[iScr]->GetTestCount();
            
#if defined (SINGLE_MODE)
            if(screens[iScr]->GetName() == SINGLE_TEST_NAME)
#endif //#if defined (SINGLE_MODE)                    
            {
                MongodbObject *oldScreenObject = CreateSubObject(screens[iScr]->GetName(), oldPlatformObject, true);
                MongodbObject *newScreenObject = dbClient->CreateObject();
                if(newScreenObject)
                {
                    newScreenObject->SetObjectName(screens[iScr]->GetName());
                    
                    for(int32 iTest = 0; iTest < count; ++iTest)
                    {
                        TestData *td = screens[iScr]->GetTestData(iTest);
                        
                        MongodbObject *testObject = CreateSubObject(td->name, oldScreenObject, false);
                        if(testObject)
                        {
                            MongodbObject *testDataObject = CreateTestDataObject(testTimeString, td);
                            if(testDataObject)
                            {
                                testObject->AddObject(testTimeString, testDataObject);
                                testObject->Finish();
                                
                                dbClient->DestroyObject(testDataObject);
                            }
                            newScreenObject->AddObject(td->name, testObject);
                            dbClient->DestroyObject(testObject);
                        }
                    }
                    
                    newScreenObject->Finish();
                    newPlatformObject->AddObject(screens[iScr]->GetName(), newScreenObject);
                    dbClient->DestroyObject(newScreenObject);
                }
                
                if(oldScreenObject)
                {
                    dbClient->DestroyObject(oldScreenObject);
                }
            }
        }

        newPlatformObject->Finish();
        dbClient->SaveObject(newPlatformObject, oldPlatformObject);    
        dbClient->DestroyObject(newPlatformObject);
        newPlatformObject = NULL;
    }
    
    if(oldPlatformObject)
    {
        dbClient->DestroyObject(oldPlatformObject);
        oldPlatformObject = NULL;
    }
}

MongodbObject * GameCore::CreateSubObject(const String &objectName, MongodbObject *dbObject, bool needFinished)
{
    MongodbObject *subObject = dbClient->CreateObject();
    if(dbObject)
    {
        bool ret = dbObject->GetSubObject(subObject, objectName, needFinished);
        if(ret)
        {
            return subObject;
        }
    }
    
    subObject->SetObjectName(objectName);
    return subObject;
}


MongodbObject * GameCore::CreateTestDataObject(const String &testTimeString, TestData *testData)
{
    MongodbObject *logObject = dbClient->CreateObject();
    if(logObject)
    {
        logObject->SetObjectName(testTimeString);
        logObject->AddString(String("Owner"), TEST_OWNER);
        logObject->AddInt64(String("TotalTime"), testData->totalTime);
        logObject->AddInt64(String("MinTime"), testData->minTime);
        logObject->AddInt64(String("MaxTime"), testData->maxTime);
        logObject->AddInt32(String("MaxTimeIndex"), testData->maxTimeIndex);
        logObject->AddInt64(String("StartTime"), testData->startTime);
        logObject->AddInt64(String("EndTime"), testData->endTime);
        
        logObject->AddInt32(String("RunCount"), testData->runCount);
        
        if(testData->runCount)
        {
            logObject->AddDouble(String("Average"), (double)testData->totalTime / (double)testData->runCount);
        }
        else 
        {
            logObject->AddDouble(String("Average"), (double)0.0f);
        }
        logObject->Finish();
    }
    return logObject;
}




