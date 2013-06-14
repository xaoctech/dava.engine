#ifndef __BEAST_COMMAND_LINE_TOOL_H__
#define __BEAST_COMMAND_LINE_TOOL_H__

#include "../CommandLineTool.h"

class BeastCommandLineTool: public CommandLineTool
{
public:

	BeastCommandLineTool();

    virtual DAVA::String GetCommandLineKey();
    virtual bool InitializeFromCommandLine();
    virtual void Process();
    virtual void PrintUsage();

    const DAVA::FilePath & GetScenePathname() const;
    
protected:

    DAVA::FilePath scenePathname;
    
};


#endif // __BEAST_COMMAND_LINE_TOOL_H__
