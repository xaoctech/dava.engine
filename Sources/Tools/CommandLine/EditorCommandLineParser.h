#ifndef __EDITOR_COMMAND_LINE_PARSER_H__
#define __EDITOR_COMMAND_LINE_PARSER_H__

#include "DAVAEngine.h"

class EditorCommandLineParser
{    
public:
    static const DAVA::int32 INVALID_POSITION = -1;

    static DAVA::int32 GetCommandsCount();

    static DAVA::String GetCommand(DAVA::uint32 commandPosition);
    static DAVA::String GetCommandParam(const DAVA::String &command);

    static bool CommandIsFound(const DAVA::String &command);
    static bool CheckPosition(DAVA::int32 commandPosition);
    
protected:
    
    static DAVA::int32 GetCommandPosition(const DAVA::String &command);
};



#endif // __EDITOR_COMMAND_LINE_PARSER_H__


