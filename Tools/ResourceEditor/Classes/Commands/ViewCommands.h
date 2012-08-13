#ifndef __VIEW_COMMANDS_H__
#define __VIEW_COMMANDS_H__

#include "Command.h"
#include "../Constants.h"

class CommandSceneInfo: public Command
{
public:	
	CommandSceneInfo();

protected:	
    
    virtual void Execute();
};


#endif // #ifndef __VIEW_COMMANDS_H__