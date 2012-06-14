#ifndef __COMMAND_LINE_TOOL_H__
#define __COMMAND_LINE_TOOL_H__

#include "DAVAEngine.h"

using namespace DAVA;

class CommandLineTool: public DAVA::Singleton<CommandLineTool>
{    
    
public:

    enum eConst
    {
        INVALID_POSITION = -1
    };

	CommandLineTool();
	virtual ~CommandLineTool();
    
    int32 CommandsCount();
    
    bool CommandIsFound(const String &command);
    int32 CommandPosition(const String &command);

    bool CheckPosition(int32 commandPosition);

protected:
    
};



#endif // __COMMAND_LINE_TOOL_H__