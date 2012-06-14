//
//  main.m
//  TemplateProjectMacOS
//
//  Created by Vitaliy  Borodovsky on 3/16/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#include "DAVAEngine.h"
#include "GameCore.h"
#include "ResourcePackerScreen.h"
#include "TexturePacker/CommandLineParser.h"

#include "SceneEditor/EditorSettings.h"


#include "SceneEditor/CommandLineTool.h"
#include "SceneEditor/SceneExporter.h"

using namespace DAVA;

#define VERSION     "0.0.3"

//void EntityTest();


void PrintUsage()
{
    printf("Usage:\n");

    printf("\t-usage or --help to display this help\n");
    printf("\t-exo - extended output\n"); 
    printf("\t-v or --verbose - detailed output\n");

    printf("\n");
    printf("resourcepacker [src_dir] - will pack resources from src_dir\n");
    
    printf("\n");
    printf("-sceneexporter [-clean [directory]] [-export [-indir [directory]] [-outdir [directory]] [-processdir [directory]] [-processfile [directory]] [-format]\n");
    printf("\t-clean - will delete all files from Data/3d/\n"); 
    printf("\t-export - will export level to Data/3d/\n"); 
    printf("\t-indir - path for Poject/DataSource/3d/ folder \n"); 
    printf("\t-outdir - path for Poject/Data/3d/ folder\n"); 
    printf("\t-processdir - foldername from DataSource/3d/ for exporting\n"); 
    printf("\t-processfile - filename from DataSource/3d/ for exporting\n"); 
    printf("\t-format - png, pvr, dxt\n"); 
    
    printf("\n");
    printf("Samples:");
    printf("-sceneexporter -clean /Users/User/Project/Data/3d/\n");
    printf("-sceneexporter -export -indir /Users/User/Project/DataSource/3d -outdir /Users/User/Project/Data/3d/ -processdir Maps/objects/\n");
    printf("-sceneexporter -export -indir /Users/User/Project/DataSource/3d -outdir /Users/User/Project/Data/3d/ -processfile Maps/level.sc2\n");
}


bool CheckPosition(int32 commandPosition)
{
    if(CommandLineTool::Instance()->CheckPosition(commandPosition))
    {
        printf("Wrong arguments\n");
        PrintUsage();

        return false;
    }
    
    return true;
}

void ProcessRecourcePacker()
{
    Vector<String> & commandLine = Core::Instance()->GetCommandLine();
    
    ResourcePackerScreen * resourcePackerScreen = new ResourcePackerScreen();
    
    String path, lastDir;
    FileSystem::SplitPath(commandLine[1], path, lastDir);
    
    resourcePackerScreen->inputGfxDirectory = FileSystem::RealPath(commandLine[1]);// "/./../../ProjectDataSource/Gfx/");
    resourcePackerScreen->outputGfxDirectory = FileSystem::RealPath(resourcePackerScreen->inputGfxDirectory + "/../../Data/" + lastDir);
    resourcePackerScreen->excludeDirectory = FileSystem::RealPath(resourcePackerScreen->inputGfxDirectory + "/../");
    
    
    String excludeDirPath, excludeDirLastDir;
    FileSystem::SplitPath(resourcePackerScreen->excludeDirectory, excludeDirPath, excludeDirLastDir);
    if (excludeDirLastDir != "DataSource")
    {
        printf("[FATAL ERROR: Packer working only inside DataSource directory]");
        return;
    }
    
    uint64 elapsedTime = SystemTimer::Instance()->AbsoluteMS();
    printf("[Resource Packer Started]\n");
    printf("[INPUT DIR] - [%s]\n", resourcePackerScreen->inputGfxDirectory.c_str());
    printf("[OUTPUT DIR] - [%s]\n", resourcePackerScreen->outputGfxDirectory.c_str());
    printf("[EXCLUDE DIR] - [%s]\n", resourcePackerScreen->excludeDirectory.c_str());
    
    
    resourcePackerScreen->PackResources();
    elapsedTime = SystemTimer::Instance()->AbsoluteMS() - elapsedTime;
    printf("[Resource Packer Compile Time: %0.3lf seconds]\n", (float64)elapsedTime / 1000.0);
    
    SafeRelease(resourcePackerScreen);
}


void FrameworkDidLaunched()
{
//	EntityTest();

    new CommandLineTool();
    new SceneExporter();
    new EditorSettings();

    printf("Running\n");
    Logger::Info("Running");

    
	if (Core::Instance()->IsConsoleMode())
	{
        if(     CommandLineTool::Instance()->CommandsCount() < 2 
           ||   (CommandLineTool::Instance()->CommandIsFound(String("-usage")))
           ||   (CommandLineTool::Instance()->CommandIsFound(String("-help")))
           )
        {
            PrintUsage();
			return;
        }
		
        if(CommandLineTool::Instance()->CommandIsFound(String("-v")) || CommandLineTool::Instance()->CommandIsFound(String("-verbose")))
        {
            CommandLineParser::Instance()->SetVerbose(true);
        }
        
        if(CommandLineTool::Instance()->CommandIsFound(String("-exo")))
        {
            CommandLineParser::Instance()->SetExtendedOutput(true);
        }
		
        if(!CommandLineTool::Instance()->CommandIsFound(String("-sceneexporter")))
        {
            ProcessRecourcePacker();
            return;  
        }
	}	
	
#if defined(__DAVAENGINE_IPHONE__)
	KeyedArchive * appOptions = new KeyedArchive();
	appOptions->SetInt32("orientation", Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT);
    
	DAVA::Core::Instance()->SetVirtualScreenSize(480, 320);
	DAVA::Core::Instance()->RegisterAvailableResourceSize(480, 320, "XGfx");
#else
	KeyedArchive * appOptions = new KeyedArchive();
//	appOptions->SetInt("width",	920);
//	appOptions->SetInt("height", 690);
    
//    int32 width = 1024;
//    int32 height = 690;
    int32 width = EditorSettings::Instance()->GetScreenWidth();
    int32 height = EditorSettings::Instance()->GetScreenHeight();

    
	appOptions->SetString("title", Format("DAVA SDK - Studio. %s", VERSION));
	appOptions->SetInt32("width",	width);
	appOptions->SetInt32("height", height);

	//appOptions->SetInt("fullscreen.width",	1280);
	//appOptions->SetInt("fullscreen.height", 800);
	
	appOptions->SetInt32("fullscreen", 0);
	appOptions->SetInt32("bpp", 32); 

	DAVA::Core::Instance()->SetVirtualScreenSize(width, height);
	DAVA::Core::Instance()->RegisterAvailableResourceSize(width, height, "XGfx");
#endif 
    
	GameCore * core = new GameCore();
	DAVA::Core::SetApplicationCore(core);
	DAVA::Core::Instance()->SetOptions(appOptions);
}


void FrameworkWillTerminate()
{
    CommandLineTool::Instance()->Release();
    SceneExporter::Instance()->Release();
    EditorSettings::Instance()->Release();
}
