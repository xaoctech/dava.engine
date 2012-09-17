#ifndef __LANDSCAPE_OPTIONS_COMMANDS_H__
#define __LANDSCAPE_OPTIONS_COMMANDS_H__

#include "Command.h"
#include "../Constants.h"

class CommandNotPassableTerrain: public Command
{
public:	
	CommandNotPassableTerrain();

protected:	
    
    virtual void Execute();
};


#endif // #ifndef __LANDSCAPE_OPTIONS_COMMANDS_H__