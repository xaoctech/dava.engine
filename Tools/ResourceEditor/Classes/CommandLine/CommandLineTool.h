#ifndef __COMMAND_LINE_TOOL_H__
#define __COMMAND_LINE_TOOL_H__

#include "DAVAEngine.h"

class CommandLineTool
{    
    
public:
    
	CommandLineTool();

    virtual DAVA::String GetCommandLineKey() = 0;
    
    virtual bool InitializeFromCommandLine() = 0;
  
    virtual void Process() = 0;

    virtual void PrintUsage() = 0;

    inline const DAVA::Set<DAVA::String> & GetErrorList() const;
    
	inline bool IsOneFrameCommand() const;

protected:
    
    DAVA::Set<DAVA::String> errors;
	bool oneFrameCommand;
};


inline const DAVA::Set<DAVA::String> & CommandLineTool::GetErrorList() const
{
    return errors;
}

inline bool CommandLineTool::IsOneFrameCommand() const
{
	return oneFrameCommand;
}


#endif // __COMMAND_LINE_TOOL_H__


