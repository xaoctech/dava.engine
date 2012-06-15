#include "CommandLineTool.h"

using namespace DAVA;

CommandLineTool::CommandLineTool()
{
    printf("CommandLine:\n");
    Logger::Info("CommandLine:");


    Vector<String> & commandLine = Core::Instance()->GetCommandLine();
    for(int32 i = 0; i < commandLine.size(); ++i)
    {
        printf("[%d] %s\n", i, commandLine[i].c_str());
        Logger::Info(Format("[%d] %s\n", i, commandLine[i].c_str()));
    }
}

CommandLineTool::~CommandLineTool()
{
    
}

bool CommandLineTool::CommandIsFound(const String &command)
{
    return (INVALID_POSITION != CommandPosition(command));
}

int32 CommandLineTool::CommandPosition(const String &command)
{
    int32 position = INVALID_POSITION;
    
    Vector<String> & commandLine = Core::Instance()->GetCommandLine();
    for(int32 i = 0; i < commandLine.size(); ++i)
    {
        if(command == commandLine[i])
        {
            position = i;
            break;
        }
    }
    
    return position;
}


int32 CommandLineTool::CommandsCount()
{
    Vector<String> & commandLine = Core::Instance()->GetCommandLine();
    return commandLine.size();
}

bool CommandLineTool::CheckPosition(int32 commandPosition)
{
    if(     (CommandLineTool::INVALID_POSITION == commandPosition) 
       ||   (CommandLineTool::Instance()->CommandsCount() < commandPosition + 2))
    {
        return false;
    }
    
    return true;
}
