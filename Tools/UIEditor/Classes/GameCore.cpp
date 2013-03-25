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

#include "HierarchyTreeController.h"
#include "PropertiesGridController.h"
#include "CommandsController.h"
#include "CopyPasteController.h"
#include "UndoRedoController.h"

#include "ScreenWrapper.h"
#include "MetadataFactory.h"
#include "EditorFontManager.h"
#include "ScreenManager.h"
#include "EditorSettings.h"
#include "ResourcesManageHelper.h"

#include <UI/UIButton.h>
#include <UI/UIStaticText.h>

using namespace DAVA;

GameCore::GameCore()
{
	new HierarchyTreeController();
    new PropertiesGridController();
    new CommandsController();
	new CopyPasteController();
    
	new ScreenWrapper();
    new MetadataFactory();
	new EditorFontManager();
	new ScreenManager();
	new EditorSettings();
	new UndoRedoController();
	
	//Initialize internal resources of application
	ResourcesManageHelper::InitInternalResources();

	tempControls.insert(new UIControl());
	tempControls.insert(new UIButton());
	tempControls.insert(new UIStaticText());
	tempControls.insert(new UIList());
	tempControls.insert(new UIScrollBar());
	tempControls.insert(new UISpinner());
}

GameCore::~GameCore()
{
	for (Set<UIControl*>::iterator iter = tempControls.begin(); iter != tempControls.end(); ++iter)
	{
		UIControl* control = (*iter);
		SafeRelease(control);
	}
}

void GameCore::OnAppStarted()
{
	cursor = 0;
	RenderManager::Instance()->SetFPS(60);

//	UIScreenManager::Instance()->RegisterScreen(0, )
//	UIScreenManager::Instance()->RegisterScreen(2, new TestScreen());
//  UIScreenManager::Instance()->SetFirst(DEFAULT_SCREEN);
//	UIScreenManager::Instance()->SetFirst(2);
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
//	if (!cursor)
//	{
//		cursor = Cursor::Create("~res:/Cursor/cursor1.png", Vector2(6, 0));
//		RenderManager::Instance()->SetCursor(cursor);
//	}
	ApplicationCore::Update(timeElapsed);
}

void GameCore::Draw()
{
	ApplicationCore::Draw();
}