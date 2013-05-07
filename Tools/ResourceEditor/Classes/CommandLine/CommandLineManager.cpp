#include "CommandLineManager.h"
#include "CommandLineTool.h"
#include "CommandLine/EditorCommandLineParser.h"

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
    
    Map<String, CommandLineTool *>::const_iterator endIT = commandLineTools.end();
    for(auto it = commandLineTools.begin(); it != endIT; ++it)
    {
        it->second->PrintUsage();
    }
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
    if(EditorCommandLineParser::CommandIsFound(String("-usage")) || EditorCommandLineParser::CommandIsFound(String("-help")))
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

bool CommandLineManager::PrintResults()
{
    if(!activeTool) return false;
    
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
    return (forceMode || 0 == errors.size());
}


