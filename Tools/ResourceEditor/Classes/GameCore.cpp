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

#include "SceneEditor/EditorSettings.h"
#include "SceneEditor/SceneValidator.h"
#include "SceneEditor/PVRConverter.h"

#include "SceneEditor/CommandLineTool.h"
#include "SceneEditor/SceneUtilsScreen.h"

#include "ImageSplitter/ImageSplitterScreen.h"

#include "TextureBrowser/TextureConvertor.h"
#include "DockParticleEditor/ParticlesEditorController.h"

using namespace DAVA;


GameCore::GameCore()
{
    virtualSize.x = Core::Instance()->GetVirtualScreenWidth();
    virtualSize.y = Core::Instance()->GetVirtualScreenHeight();
}

GameCore::~GameCore()
{
	
}

void GameCore::OnAppStarted()
{
	Logger::Instance()->SetLogFilename(FilePath("ResEditor.txt"));
	RenderManager::Instance()->SetFPS(30);

    LocalizationSystem::Instance()->SetCurrentLocale(EditorSettings::Instance()->GetLanguage());
	LocalizationSystem::Instance()->InitWithDirectory("~res:/Strings");

    
#ifdef __DAVAENGINE_BEAST__
	new BeastProxyImpl();
#else 
    new BeastProxy();
#endif //__DAVAENGINE_BEAST__
	
	new PVRConverter();

#if defined (__DAVAENGINE_MACOS__)
	PVRConverter::Instance()->SetPVRTexTool(String("~res:/PVRTexToolCL"));
#elif defined (__DAVAENGINE_WIN32__)
    PVRConverter::Instance()->SetPVRTexTool(String("~res:/PVRTexToolCL.exe"));
#endif

	resourcePackerScreen = new ResourcePackerScreen();
    sceneEditorScreenMain = new SceneEditorScreenMain();
    sceneUtilsScreen = new SceneUtilsScreen();

	new ParticlesEditorController();
    imageSplitterScreen = new ImageSplitterScreen();

	UIScreenManager::Instance()->RegisterScreen(SCREEN_RESOURCE_PACKER, resourcePackerScreen);
    UIScreenManager::Instance()->RegisterScreen(SCREEN_SCENE_EDITOR_MAIN, sceneEditorScreenMain);
    UIScreenManager::Instance()->RegisterScreen(SCREEN_SCENE_UTILS, sceneUtilsScreen);
    UIScreenManager::Instance()->RegisterScreen(SCREEN_IMAGE_SPLITTER, imageSplitterScreen);

    
    if(     CommandLineTool::Instance()
       &&   (CommandLineTool::Instance()->CommandIsFound(String("-sceneexporter"))
       ||   CommandLineTool::Instance()->CommandIsFound(String("-scenesaver"))))
    {
        UIScreenManager::Instance()->SetFirst(SCREEN_SCENE_UTILS);
    }
    else if(CommandLineTool::Instance() && CommandLineTool::Instance()->CommandIsFound(String("-imagesplitter")))
    {
        UIScreenManager::Instance()->SetFirst(SCREEN_IMAGE_SPLITTER);
    }
    else
    {
        UIScreenManager::Instance()->SetFirst(SCREEN_SCENE_EDITOR_MAIN);
        Texture::SetDefaultFileFormat((ImageFileFormat)EditorSettings::Instance()->GetTextureViewFileFormat());
    }
	
	// Yuri Coder, 2013/01/23. The call below is needed for Win32 linker to notify it we are using
	// ParticleEffectNode and thus REGISTER_CLASS(ParticleEffectNode) must not be removed during optimization.
	ParticleEffectNode* effectNode = new ParticleEffectNode();
	delete effectNode;
}

void GameCore::OnAppFinished()
{
	PVRConverter::Instance()->Release();
    SceneValidator::Instance()->Release();

	BeastProxy::Instance()->Release();

	SafeRelease(resourcePackerScreen);
    SafeRelease(sceneEditorScreenMain);
    SafeRelease(sceneUtilsScreen);
    SafeRelease(imageSplitterScreen);
}

void GameCore::OnSuspend()
{
	//prevent going to suspend
    //ApplicationCore::OnSuspend();
}

void GameCore::OnResume()
{
    ApplicationCore::OnResume();
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
    Vector2 newVirtualSize(Core::Instance()->GetVirtualScreenWidth(), Core::Instance()->GetVirtualScreenHeight());
    
    if(virtualSize != newVirtualSize)
    {
        virtualSize = newVirtualSize;
        ResizeScreens();
    }
    
	ApplicationCore::Update(timeElapsed);
}

void GameCore::Draw()
{
	ApplicationCore::Draw();

}

void GameCore::ResizeScreens()
{
    if(sceneEditorScreenMain)
    {
        sceneEditorScreenMain->SetSize(virtualSize);
    }
}


