#ifndef __COMMAND_LINE_MANAGER_H__
#define __COMMAND_LINE_MANAGER_H__

#include "DAVAEngine.h"

class CommandLineTool;
class CommandLineManager: public DAVA::Singleton<CommandLineManager>
{    
public:
	CommandLineManager();
	virtual ~CommandLineManager();
    
    bool IsCommandLineModeEnabled() { return isCommandLineModeEnabled; };
    
    void Process();
    void PrintResults();
    
protected:

    void AddCommandLineTool(CommandLineTool *tool);
    
    void ParseCommandLine();
    void PrintUsage();
    
    void DetectCommandLineMode();
    
    void FindActiveTool();
    
    DAVA::Map<DAVA::String, CommandLineTool *> commandLineTools;
    bool isCommandLineModeEnabled;
    
    CommandLineTool *activeTool;
};



#endif // __COMMAND_LINE_MANAGER_H__

