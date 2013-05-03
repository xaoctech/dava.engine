//
//  main.m
//  TemplateProjectMacOS
//
//  Created by Vitaliy  Borodovsky on 3/16/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#include "DAVAEngine.h"
#include "GameCore.h"
#include "TexturePacker/CommandLineParser.h"
#include "TexturePacker/ResourcePacker2D.h"

#include "SceneEditor/EditorSettings.h"
#include "SceneEditor/EditorConfig.h"
#include "SceneEditor/SceneValidator.h"

#include "SceneEditor/CommandLineTool.h"
#include "SceneEditor/SceneExporter.h"
#include "SceneEditor/SceneSaver.h"

#include "PVRConverter.h"
#include "version.h"

using namespace DAVA;

 
void PrintUsage()
{
    printf("Usage:\n");

    printf("\t-usage or --help to display this help\n");
    printf("\t-exo - extended output\n"); 
    printf("\t-v or --verbose - detailed output\n");

    printf("\n");
    printf("resourcepacker [src_dir] - will pack resources from src_dir\n");
    
    printf("\n");
    printf("-sceneexporter [-clean [directory]] [-export [-indir [directory]] [-outdir [directory]] [-processdir [directory]] [-processfile [directory]] [-format] [-forceclose]\n");
    printf("\t-clean - will delete all files from Data/3d/\n"); 
    printf("\t-export - will export level to Data/3d/\n"); 
    printf("\t-indir - path for Poject/DataSource/3d/ folder \n"); 
    printf("\t-outdir - path for Poject/Data/3d/ folder\n"); 
    printf("\t-processdir - foldername from DataSource/3d/ for exporting\n"); 
    printf("\t-processfile - filename from DataSource/3d/ for exporting\n"); 
    printf("\t-format - png, pvr, dxt\n"); 
    printf("\t-forceclose - to don't display error dialogs");

    printf("\n");
    printf("-scenesaver [-clean [directory]] [-save [-indir [directory]] [-outdir [directory]] [-processfile [directory]] [-forceclose]\n");
	printf("-scenesaver [-resave [-indir [directory]] [-processfile [directory]] [-forceclose]\n");
    printf("\t-clean - will delete all files from Data/3d/\n");
    printf("\t-save - will save level to selected Data/3d/\n");
	printf("\t-resave - will open and save level\n");
    printf("\t-indir - path for Poject/DataSource/3d/ folder \n");
    printf("\t-outdir - path for Poject/Data/3d/ folder\n");
    printf("\t-processfile - filename from DataSource/3d/ for saving\n");
    printf("\t-forceclose - to don't display error dialogs");

    
    printf("\n");
    printf("Samples:");
    printf("-sceneexporter -clean /Users/User/Project/Data/3d/\n");
    printf("-sceneexporter -export -indir /Users/User/Project/DataSource/3d -outdir /Users/User/Project/Data/3d/ -processdir Maps/objects/\n");
    printf("-sceneexporter -export -indir /Users/User/Project/DataSource/3d -outdir /Users/User/Project/Data/3d/ -processfile Maps/level.sc2 -forceclose\n");
    
    printf("\n");
    printf("-scenesaver -clean /Users/User/Project/Data/3d/\n");
    printf("-scenesaver -save -indir /Users/User/Project/DataSource/3d -outdir /Users/User/Project/Data/3d/ -processfile Maps/level.sc2 -forceclose\n");
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
    
    ResourcePacker2D * resourcePacker = new ResourcePacker2D();
    
    FilePath commandLinePath(commandLine[1]);
    commandLinePath.MakeDirectoryPathname();
    
    FilePath lastDir(commandLinePath.GetDirectory().GetLastDirectoryName());
    lastDir.MakeDirectoryPathname();
    
    FilePath outputh = commandLinePath + "../../Data/" + lastDir;
    
    resourcePacker->InitFolders(commandLinePath, outputh);
    
    if(resourcePacker->excludeDirectory.IsEmpty())
    {
        printf("[FATAL ERROR: Packer has wrong input pathname]");
        return;
    }
    
    if (resourcePacker->excludeDirectory.GetLastDirectoryName() != "DataSource")
    {
        printf("[FATAL ERROR: Packer working only inside DataSource directory]");
        return;
    }
    
    new PVRConverter();
    
    
    if(commandLine.size() < 3)
    {
        printf("[FATAL ERROR: PVRTexTool path need to be second parameter]");
        return;
    }
    
#if defined (__DAVAENGINE_MACOS__)
	String toolName = String("/PVRTexToolCL");
#elif defined (__DAVAENGINE_WIN32__)
	String toolName = String("/PVRTexToolCL.exe");
#endif
    PVRConverter::Instance()->SetPVRTexTool(resourcePacker->excludeDirectory + commandLine[2] + toolName);
    
    uint64 elapsedTime = SystemTimer::Instance()->AbsoluteMS();
    printf("[Resource Packer Started]\n");
    printf("[INPUT DIR] - [%s]\n", resourcePacker->inputGfxDirectory.GetAbsolutePathname().c_str());
    printf("[OUTPUT DIR] - [%s]\n", resourcePacker->outputGfxDirectory.GetAbsolutePathname().c_str());
    printf("[EXCLUDE DIR] - [%s]\n", resourcePacker->excludeDirectory.GetAbsolutePathname().c_str());
    
    Texture::InitializePixelFormatDescriptors();
    resourcePacker->PackResources();
    elapsedTime = SystemTimer::Instance()->AbsoluteMS() - elapsedTime;
    printf("[Resource Packer Compile Time: %0.3lf seconds]\n", (float64)elapsedTime / 1000.0);
    
    PVRConverter::Instance()->Release();
    
    SafeDelete(resourcePacker);
}


/*
    Is it possible, to store size before vector, seems doable and cache friendly, because to tranverse vector firstly you 
    need to read size.
 */
template<class T>
class CacheFriendlyVector
{
public:
    uint8 * data;
};



class EntityX
{
//    Vector<Component*> fastAccessArray;
//    Vector<Component*> allComponents;
    Component ** fastAccessArray;
    Component ** allComponents;
    EntityX ** children;
};


class EntitySTL
{
    CacheFriendlyVector<Component*> fastAccessArray;
    CacheFriendlyVector<Component*> allComponents;
    CacheFriendlyVector<EntitySTL*> children;
};

/*
    => 24 bytes for each entity. (8 * 3 = 24 = 64bit system) (12bytes on 32 bit system)
    =>
    
    Action update transform: (тут не может не быть cache misses)
 
    Entity * entity = GetEntity(); // cache miss
    TransformComponent * tc = entity->fastTransforms[COMPONENT_TRANSFORM]; // cache miss
    tc->UpdateLocalTransform(transform); // cache miss
 
 
    TransformSystem:
    for each object go to his children and add them, for them do the same, linearly
 
 */



void FrameworkDidLaunched()
{
    uint32 size = sizeof(EntityX);
    uint32 size2 = sizeof(EntitySTL);
    
    new CommandLineTool();
    new SceneExporter();
    new SceneSaver();
    new EditorSettings();
	new EditorConfig();
    new SceneValidator();

    SceneValidator::Instance()->SetPathForChecking(EditorSettings::Instance()->GetProjectPath());
    
    FilePath dataSourcePathname = EditorSettings::Instance()->GetDataSourcePath();
    String sourceFolder = String("DataSource/3d");
    
    if(!CommandLineTool::Instance()->CommandIsFound(String("-imagesplitter")))
    {
        if(sourceFolder.length() <= dataSourcePathname.GetAbsolutePathname().length())
        {
            uint64 creationTime = SystemTimer::Instance()->AbsoluteMS();
            SceneValidator::Instance()->CreateDefaultDescriptors(dataSourcePathname);
            creationTime = SystemTimer::Instance()->AbsoluteMS() - creationTime;
//            Logger::Info("[CreateDefaultDescriptors time is %ldms]", creationTime);
        }
    }
    
    
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
		
        if(     !CommandLineTool::Instance()->CommandIsFound(String("-sceneexporter"))
           &&   !CommandLineTool::Instance()->CommandIsFound(String("-imagesplitter"))
           &&   !CommandLineTool::Instance()->CommandIsFound(String("-scenesaver"))
           )
        {
            ProcessRecourcePacker();
            return;  
        }
	}	
	
#if defined(__DAVAENGINE_IPHONE__)
	KeyedArchive * appOptions = new KeyedArchive();
	appOptions->SetInt32("orientation", Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT);
    
	DAVA::Core::Instance()->SetVirtualScreenSize(480, 320);
	DAVA::Core::Instance()->RegisterAvailableResourceSize(480, 320, "Gfx");
#else
	KeyedArchive * appOptions = new KeyedArchive();
    
    int32 width = (int32)DAVA::Core::Instance()->GetVirtualScreenWidth();
    int32 height = (int32)DAVA::Core::Instance()->GetVirtualScreenHeight();
    if(width <= 0 || height <= 0)
    {
        width = EditorSettings::Instance()->GetScreenWidth();
        height = EditorSettings::Instance()->GetScreenHeight();

        DAVA::Core::Instance()->SetVirtualScreenSize(width, height);
    }
    
	appOptions->SetString("title", Format("dava framework - resource editor | %s", RESOURCE_EDITOR_VERSION));
	appOptions->SetInt32("width",	width);
	appOptions->SetInt32("height", height);

	appOptions->SetInt32("fullscreen", 0);
	appOptions->SetInt32("bpp", 32); 

	DAVA::Core::Instance()->RegisterAvailableResourceSize(width, height, "Gfx");
#endif
    
	GameCore * core = new GameCore();
	DAVA::Core::SetApplicationCore(core);
	DAVA::Core::Instance()->SetOptions(appOptions);
    DAVA::Core::Instance()->EnableReloadResourceOnResize(false);

	SafeRelease(appOptions);
}


void FrameworkWillTerminate()
{
	SceneValidator::Instance()->Release();
	EditorConfig::Instance()->Release();
	EditorSettings::Instance()->Release();
    SceneExporter::Instance()->Release();
    SceneSaver::Instance()->Release();

	CommandLineTool::Instance()->Release();
}
