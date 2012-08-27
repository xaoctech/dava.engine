/*
 *  GameCore.cpp
 *  TemplateProjectMacOS
 *
 *  Created by Vitaliy  Borodovsky on 3/19/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "GameCore.h"
#include "AppScreens.h"
#include "ResourcePackerScreen.h"
#include "SceneEditor/SceneEditorScreenMain.h"

#ifdef __DAVAENGINE_BEAST__
#include "BeastProxyImpl.h"
#else
#include "BeastProxy.h"
#endif //__DAVAENGINE_BEAST__

#include "SceneEditor/OutputManager.h"
#include "SceneEditor/EditorSettings.h"
#include "SceneEditor/SceneValidator.h"
#include "SceneEditor/PVRConverter.h"
#include "SceneEditor/PVRUtils.h"

#include "SceneEditor/CommandLineTool.h"
#include "SceneEditor/ExporterScreen.h"

#if defined (DAVA_QT)
#include "Qt/SceneDataManager.h"
#endif //#if defined (DAVA_QT)


using namespace DAVA;


GameCore::GameCore()
{
#if defined (DAVA_QT)
    virtualSize.x = Core::Instance()->GetVirtualScreenWidth();
    virtualSize.y = Core::Instance()->GetVirtualScreenHeight();
#endif //#if defined (DAVA_QT)
}

GameCore::~GameCore()
{
	
}

void GameCore::OnAppStarted()
{
	Logger::Instance()->SetLogFilename("ResEditor.txt");
	RenderManager::Instance()->SetFPS(30);
    
    Stats::Instance()->EnableStatsOutputEventNFrame(30);

    LocalizationSystem::Instance()->SetCurrentLocale(EditorSettings::Instance()->GetLanguage());
	LocalizationSystem::Instance()->InitWithDirectory("~res:/Strings");

    
#ifdef __DAVAENGINE_BEAST__
	new BeastProxyImpl();
#else 
    new BeastProxy();
#endif //__DAVAENGINE_BEAST__
	
    new OutputManager();
	new PVRConverter();
    new PVRUtils();
#if defined (DAVA_QT)
    new SceneDataManager();
#endif //#if defined (DAVA_QT)
        
    
	resourcePackerScreen = new ResourcePackerScreen();
    sceneEditorScreenMain = new SceneEditorScreenMain();
    
    exporterScreen = new ExporterScreen();

	UIScreenManager::Instance()->RegisterScreen(SCREEN_RESOURCE_PACKER, resourcePackerScreen);
    UIScreenManager::Instance()->RegisterScreen(SCREEN_SCENE_EDITOR_MAIN, sceneEditorScreenMain);
    
    UIScreenManager::Instance()->RegisterScreen(SCREEN_EXPORTER, exporterScreen);

    if(CommandLineTool::Instance() && CommandLineTool::Instance()->CommandIsFound(String("-sceneexporter")))
    {
        UIScreenManager::Instance()->SetFirst(SCREEN_EXPORTER);
    }
    else 
    {
        UIScreenManager::Instance()->SetFirst(SCREEN_SCENE_EDITOR_MAIN);
    }
}

void GameCore::OnAppFinished()
{
#if defined (DAVA_QT)
    SceneDataManager::Instance()->Release();
#endif //#if defined (DAVA_QT)

    PVRUtils::Instance()->Release();
	PVRConverter::Instance()->Release();
    OutputManager::Instance()->Release();
    SceneValidator::Instance()->Release();
    
	SafeRelease(resourcePackerScreen);
    SafeRelease(sceneEditorScreenMain);
    SafeRelease(exporterScreen);
}

void GameCore::OnSuspend()
{
	//prevent going to suspend
    //ApplicationCore::OnSuspend();
}

void GameCore::OnResume()
{
    ApplicationCore::OnResume();
    
    SceneValidator::Instance()->ReloadTextures();
    sceneEditorScreenMain->RecreteFullTilingTexture();
}

void GameCore::OnBackground()
{
	//ApplicationCore::OnBackground();
}

void GameCore::BeginFrame()
{
	ApplicationCore::BeginFrame();
	RenderManager::Instance()->ClearWithColor(0, 0, 0, 0);
}

void GameCore::Update(float32 timeElapsed)
{
#if defined (DAVA_QT)
    Vector2 newVirtualSize(Core::Instance()->GetVirtualScreenWidth(), Core::Instance()->GetVirtualScreenHeight());
    
    if(virtualSize != newVirtualSize)
    {
        virtualSize = newVirtualSize;
        ResizeScreens();
    }

#endif //#if defined (DAVA_QT)
    
	ApplicationCore::Update(timeElapsed);
}

void GameCore::Draw()
{
	ApplicationCore::Draw();

}

#if defined (DAVA_QT)
void GameCore::ResizeScreens()
{
    if(sceneEditorScreenMain)
    {
        sceneEditorScreenMain->SetSize(virtualSize);
    }
}

#endif //#if defined (DAVA_QT)
