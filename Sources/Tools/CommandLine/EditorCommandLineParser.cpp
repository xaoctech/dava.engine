#include "EditorCommandLineParser.h"

using namespace DAVA;

bool EditorCommandLineParser::CommandIsFound(const DAVA::String &command)
{
    return (INVALID_POSITION != GetCommandPosition(command));
}

DAVA::String EditorCommandLineParser::GetCommand(DAVA::uint32 commandPosition)
{
    Vector<String> & commandLine = Core::Instance()->GetCommandLine();
    DVASSERT(commandPosition < commandLine.size());

    return commandLine[commandPosition];
}


DAVA::int32 EditorCommandLineParser::GetCommandPosition(const DAVA::String &command)
{
    int32 position = INVALID_POSITION;
    
    Vector<String> & commandLine = Core::Instance()->GetCommandLine();
    for(int32 i = 0; i < (int32)commandLine.size(); ++i)
    {
        if(command == commandLine[i])
        {
            position = i;
            break;
        }
    }
    
    return position;
}

DAVA::String EditorCommandLineParser::GetCommandParam(const DAVA::String &command)
{
    int32 commandPosition = GetCommandPosition(command);
    if(EditorCommandLineParser::CheckPosition(commandPosition))
    {
        return EditorCommandLineParser::GetCommand(commandPosition + 1);
    }

    return String();
}


int32 EditorCommandLineParser::GetCommandsCount()
{
    Vector<String> & commandLine = Core::Instance()->GetCommandLine();
    return commandLine.size();
}

bool EditorCommandLineParser::CheckPosition(int32 commandPosition)
{
    if(     (INVALID_POSITION == commandPosition)
       ||   (GetCommandsCount() < commandPosition + 2))
    {
        return false;
    }
    
    return true;
}
