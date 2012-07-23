#ifndef __COMMAND_VIEWPORT_H__
#define __COMMAND_VIEWPORT_H__

#include "Command.h"
#include "../Constants.h"

class CommandViewport: public Command
{
public:	
	CommandViewport(ResourceEditor::eViewportType type);
    
protected:
    virtual void Execute();

protected:
    ResourceEditor::eViewportType viewportType;
};




#endif // #ifndef __COMMAND_VIEWPORT_H__