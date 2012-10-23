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
#include "TestScreen.h"
#include "FileManagerWrapper.h"
#include "ScreenShotHelper.h"
#include "SettingsManager.h"

using namespace DAVA;

GameCore::GameCore()
{
	m_pResultScreen = NULL;
}

GameCore::~GameCore()
{
	
}

void GameCore::OnAppStarted()
{
	SettingsManager::Instance()->Init();
	
	m_pCursor = 0;
	RenderManager::Instance()->SetFPS(60);

	String dirPath = "~res:/Maps/";
	Vector<String> v = FileManagerWrapper::GetFileListByExtension(dirPath, ".sc2");
	v.push_back("summer_level.sc2");

	for(Vector<String>::const_iterator it = v.begin(); it != v.end(); ++it) {
		Logger::Debug("%s", (*it).c_str());
	}
	
	for(Vector<String>::const_iterator it = v.begin(); it != v.end(); ++it) {
		Test *test = new Test(dirPath + (*it));
		if(test != NULL) {
			UIScreenManager::Instance()->RegisterScreen(test->GetScreenId(), test);
			m_Tests.push_back(test);
		}
	}

	if(v.size() > 0) {
		m_bAppFinished = false;
		
		m_nTestCount = m_Tests.size();
		Test *firstTest = m_Tests.front();
		UIScreenManager::Instance()->SetFirst(firstTest->GetScreenId());
	} else {
		m_bAppFinished = true;
	}
}

void GameCore::OnAppFinished()
{
	SafeRelease(m_pCursor);
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
	
	if(!m_bAppFinished) {
		Test *curTest = m_Tests.front();
		if(!curTest->IsFinished()) {
			curTest->Update(timeElapsed);
		} else {
			if(m_pResultScreen == NULL) {
				int testNum = m_nTestCount - m_Tests.size() + 1;
				m_pResultScreen = new ResultScreen(curTest->GetStat(), curTest->GetFileName(), m_nTestCount, testNum);
				UIScreenManager::Instance()->RegisterScreen(1, m_pResultScreen);
				UIScreenManager::Instance()->SetFirst(1);
			}
			
			if(m_pResultScreen->IsFinished()) {
				SafeRelease(m_pResultScreen);
				m_Tests.pop_front();

				if(m_Tests.size() == 0) {
					m_bAppFinished = true;
				} else {
					Test *newCurTest = m_Tests.front();
					if(newCurTest != NULL) {
						UIScreenManager::Instance()->SetFirst(newCurTest->GetScreenId());
					}
				}

				SafeRelease(curTest);
			}
		}
	} else {
		exit(0);
	}
}

void GameCore::Draw()
{
	ApplicationCore::Draw();
}