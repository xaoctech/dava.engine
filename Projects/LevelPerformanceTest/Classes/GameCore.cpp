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
#include "AppScreens.h"
#include "FileManagerWrapper.h"
#include "SettingsManager.h"
#include "LandscapeTestData.h"

using namespace DAVA;

GameCore::GameCore()
{
	resultScreen = NULL;
}

GameCore::~GameCore()
{
	
}

void GameCore::OnAppStarted()
{
	SettingsManager::Instance()->InitWithFile("~res:/Config/config.yaml");
	
	cursor = 0;
	RenderManager::Instance()->SetFPS(60);

	String dirPath = "~res:/Maps/";
	Vector<String> v = FileManagerWrapper::GetFileListByExtension(dirPath, ".sc2");
	v.push_back("a_level.sc2");

	for(Vector<String>::const_iterator it = v.begin(); it != v.end(); ++it)
    {
		Test *test = new Test(dirPath + (*it));
		if(test != NULL)
        {
			UIScreenManager::Instance()->RegisterScreen(test->GetScreenId(), test);
			tests.push_back(test);
		}
	}

	if(v.size() > 0)
    {
		appFinished = false;
		
		testCount = tests.size();
		Test *firstTest = tests.front();
		UIScreenManager::Instance()->SetFirst(firstTest->GetScreenId());
	}
    else
    {
		appFinished = true;
	}
}

void GameCore::OnAppFinished()
{
	SafeRelease(cursor);
}

void GameCore::OnSuspend()
{
    ApplicationCore::OnSuspend();
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
	RenderManager::Instance()->ClearWithColor(0, 0, 0, 0);
}

void GameCore::Update(float32 timeElapsed)
{
	ApplicationCore::Update(timeElapsed);
	
	if(!appFinished)
    {
		Test *curTest = tests.front();
        if(curTest->IsFinished())
        {
			if(resultScreen == NULL)
            {
				resultScreen = new ResultScreen(curTest->GetLandscapeTestData(),
												curTest->GetFileName(),
												curTest->GetLandscapeTexture());
                
				UIScreenManager::Instance()->RegisterScreen(RESULT_SCREEN, resultScreen);
                UIScreenManager::Instance()->SetScreen(RESULT_SCREEN);
			}
			
			if(resultScreen->IsFinished())
            {
				SafeRelease(resultScreen);
				tests.pop_front();

				if(tests.size() == 0)
                {
					appFinished = true;
				}
                else
                {
					Test *newCurTest = tests.front();
					if(newCurTest != NULL)
                    {
						UIScreenManager::Instance()->SetScreen(newCurTest->GetScreenId());
					}
				}

				SafeRelease(curTest);
			}
		}
	}
    else
    {
		exit(0);
	}
}

void GameCore::Draw()
{
	ApplicationCore::Draw();
}
