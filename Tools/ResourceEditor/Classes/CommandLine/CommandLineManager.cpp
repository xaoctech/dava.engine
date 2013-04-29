#include "CommandLineManager.h"
#include "CommandLineTool.h"
#include "EditorCommandLineParser.h"

#include "ImageSplitter/ImageSplitterTool.h"
#include "SceneUtils/CleanFolderTool.h"
#include "SceneSaver/SceneSaverTool.h"
#include "SceneExporter/SceneExporterTool.h"

#include "TexturePacker/CommandLineParser.h"

#include "../Qt/Main/QtUtils.h"


using namespace DAVA;

void CommandLineManager::PrintUsage()
{
    printf("Usage:\n");
    
    printf("\t-usage or --help to display this help\n");
    printf("\t-exo - extended output\n");
    printf("\t-v or --verbose - detailed output\n");
    printf("\t-forceclose - close editor after job would be finished\n");
    
    printf("\n");
    printf("-sceneexporter [-indir [directory]] [-outdir [directory]] [-processdir [directory]] [-processfile [directory]] [-format]\n");
    printf("\twill export scene file from DataSource/3d to Data/3d\n");
    printf("\t-indir - path for Poject/DataSource/3d/ folder \n");
    printf("\t-outdir - path for Poject/Data/3d/ folder\n");
    printf("\t-processdir - foldername from DataSource/3d/ for exporting\n");
    printf("\t-processfile - filename from DataSource/3d/ for exporting\n");
    printf("\t-format - png, pvr, dxt\n");
    
    printf("\n");
    printf("-scenesaver -save [-indir [directory]] [-outdir [directory]] [-processfile [directory]]\n");
    printf("-scenesaver -resave [-indir [directory]] [-processfile [directory]] [-forceclose]\n");
    printf("\twill save scene file from DataSource/3d to any Data or DataSource folder\n");
    printf("\t-save - will save level to selected Data/3d/\n");
    printf("\t-resave - will open and save level\n");
    printf("\t-indir - path for Poject/DataSource/3d/ folder \n");
    printf("\t-outdir - path for Poject/Data/3d/ folder\n");
    printf("\t-processfile - filename from DataSource/3d/ for saving\n");

    printf("\n");
    printf("-cleanfolder [-folder [directory]]\n");
    printf("\twill delete folder with files \n");
    printf("\t-folder - path for /Users/User/Project/Data/3d/ folder \n");

    printf("\n");
    printf("-imagesplitter -split [-file [file]]\n");
    printf("-imagesplitter -merge [-folder [directory]]\n");
    printf("\twill split one image at four channels or merge four channels to one image\n");
    printf("\t-file - filename of the splitting file\n");
    printf("\t-folder - path for folder with four channels\n");

    printf("\n");
    printf("Samples:");
    printf("-sceneexporter -export -indir /Users/User/Project/DataSource/3d -outdir /Users/User/Project/Data/3d/ -processdir Maps/objects/\n");
    printf("-sceneexporter -export -indir /Users/User/Project/DataSource/3d -outdir /Users/User/Project/Data/3d/ -processfile Maps/level.sc2 -forceclose\n");
    
    printf("\n");
    printf("-scenesaver -save -indir /Users/User/Project/DataSource/3d -outdir /Users/User/Project/Data/3d/ -processfile Maps/level.sc2 -forceclose\n");
    printf("-scenesaver -resave -indir /Users/User/Project/DataSource/3d -processfile Maps/level.sc2 -forceclose\n");

    printf("\n");
    printf("-cleanfolder -folder /Users/User/Project/Data/3d -forceclose\n");

    printf("\n");
    printf("-imagesplitter -split -file /Users/User/Project/Data/3d/image.png\n");
    printf("-imagesplitter -merge -folder /Users/User/Project/Data/3d/\n");
}


CommandLineManager::CommandLineManager()
{
    AddCommandLineTool(new CleanFolderTool());
    AddCommandLineTool(new ImageSplitterTool());
    AddCommandLineTool(new SceneExporterTool());
    AddCommandLineTool(new SceneSaverTool());
 
    ParseCommandLine();
    
    DetectCommandLineMode();

    FindActiveTool();
    
    if(activeTool)
    {
        activeTool->InitializeFromCommandLine();
    }
}

CommandLineManager::~CommandLineManager()
{
    Map<String, CommandLineTool *>::const_iterator endIT = commandLineTools.end();
    for(auto it = commandLineTools.begin(); it != endIT; ++it)
    {
        SafeDelete(it->second);
    }
    commandLineTools.clear();
}

void CommandLineManager::AddCommandLineTool(CommandLineTool *tool)
{
    DVASSERT(tool);
    
    if(commandLineTools.find(tool->GetCommandLineKey()) != commandLineTools.end())
    {
        SafeDelete(commandLineTools[tool->GetCommandLineKey()]);
    }
    
    commandLineTools[tool->GetCommandLineKey()] = tool;
}

void CommandLineManager::ParseCommandLine()
{
    if(     EditorCommandLineParser::GetCommandsCount() < 2
       ||   (EditorCommandLineParser::CommandIsFound(String("-usage")))
       ||   (EditorCommandLineParser::CommandIsFound(String("-help")))
       )
    {
        PrintUsage();
        return;
    }
    
    if(EditorCommandLineParser::CommandIsFound(String("-v")) || EditorCommandLineParser::CommandIsFound(String("-verbose")))
    {
        CommandLineParser::Instance()->SetVerbose(true);
    }
    
    if(EditorCommandLineParser::CommandIsFound(String("-exo")))
    {
        CommandLineParser::Instance()->SetExtendedOutput(true);
    }
}

void CommandLineManager::DetectCommandLineMode()
{
    isCommandLineModeEnabled = Core::Instance()->IsConsoleMode();
    
    Map<String, CommandLineTool *>::const_iterator endIT = commandLineTools.end();
    for(auto it = commandLineTools.begin(); it != endIT; ++it)
    {
        if(EditorCommandLineParser::CommandIsFound(it->first))
        {
            isCommandLineModeEnabled = true;
            break;
        }
    }
}


void CommandLineManager::FindActiveTool()
{
    activeTool = NULL;
    if(isCommandLineModeEnabled)
    {
        Map<String, CommandLineTool *>::const_iterator endIT = commandLineTools.end();
        for(auto it = commandLineTools.begin(); it != endIT; ++it)
        {
            if(EditorCommandLineParser::CommandIsFound(it->first))
            {
                activeTool = it->second;
                break;
            }
        }
    }
}

void CommandLineManager::Process()
{
    if(activeTool)
    {
        activeTool->Process();
    }
}

void CommandLineManager::PrintResults()
{
    if(!activeTool) return;
    
    const Set<String> &errors = activeTool->GetErrorList();
    if(0 < errors.size())
    {
        printf("\nErrors:\n");
        Logger::Error("Errors:");
        Set<String>::const_iterator endIt = errors.end();
        int32 index = 0;
        for (auto it = errors.begin(); it != endIt; ++it)
        {
            printf("[%d] %s\n", index, (*it).c_str());
            Logger::Error(Format("[%d] %s\n", index, (*it).c_str()));
            
            ++index;
        }
        
        ShowErrorDialog(errors);
    }
    
    bool forceMode =    EditorCommandLineParser::CommandIsFound(String("-force"))
                    ||  EditorCommandLineParser::CommandIsFound(String("-forceclose"));
    if(forceMode || 0 == errors.size())
    {
        Core::Instance()->Quit();
    }
}


