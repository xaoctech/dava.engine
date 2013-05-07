#ifndef __COMMAND_LINE_TOOL_H__
#define __COMMAND_LINE_TOOL_H__

#include "DAVAEngine.h"

class CommandLineTool
{    
    
public:
    
    virtual DAVA::String GetCommandLineKey() = 0;
    
    virtual bool InitializeFromCommandLine() = 0;
  
    virtual void Process() = 0;

    virtual void PrintUsage() = 0;

    inline const DAVA::Set<DAVA::String> & GetErrorList() const;
    
protected:
    
    DAVA::Set<DAVA::String> errors;
};


inline const DAVA::Set<DAVA::String> & CommandLineTool::GetErrorList() const
{
    return errors;
}



#endif // __COMMAND_LINE_TOOL_H__


