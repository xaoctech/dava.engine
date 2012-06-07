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
#include "SpriteTest.h"
#include "LandscapeTest.h"
#include "MongodbTest.h"
#include "CacheTest.h"

using namespace DAVA;

GameCore::GameCore()
{
    logFile = NULL;
    currentScreenIndex = 0;
}

GameCore::~GameCore()
{
}

void GameCore::OnAppStarted()
{
	RenderManager::Instance()->SetFPS(60);

    CreateLogFile();

    new MongodbTest();
    new CacheTest("Cache Test");
    new LandscapeTest("Landscape Textures Test", LandscapeNode::TILED_MODE_COUNT);
    new LandscapeTest("Landscape Mixed Mode", LandscapeNode::TILED_MODE_MIXED);
    new LandscapeTest("Landscape Tiled Mode", LandscapeNode::TILED_MODE_TILEMASK);
    new LandscapeTest("Landscape Texture Mode", LandscapeNode::TILED_MODE_TEXTURE);
    new SpriteTest();
    
    currentScreenIndex = 0;
    RunCurrentTest();
}

void GameCore::RegisterScreen(DAVA::UIScreen *screen)
{
    screens.push_back(screen);
    UIScreenManager::Instance()->RegisterScreen(screens.size(), screen);
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
	ApplicationCore::Update(timeElapsed);
}

void GameCore::Draw()
{
	ApplicationCore::Draw();
}

void GameCore::TestFinished()
{
    RunCurrentTest();
}

void GameCore::RunCurrentTest()
{
    if(0 == currentScreenIndex)
    {
        UIScreenManager::Instance()->SetFirst(currentScreenIndex + 1);
        ++currentScreenIndex;
    }
    else if(currentScreenIndex < screens.size())
    {
        UIScreenManager::Instance()->SetScreen(currentScreenIndex + 1);
        ++currentScreenIndex;
    }
    else 
    {
		Core::Instance()->Quit();
    }
}


