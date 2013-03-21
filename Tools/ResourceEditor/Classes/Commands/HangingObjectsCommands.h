#ifndef __RESOURCE_EDITOR_HANGING_OBJECTS_COMMANDS_H__
#define __RESOURCE_EDITOR_HANGING_OBJECTS_COMMANDS_H__

#include "Command.h"
#include "DAVAEngine.h"
#include "../Constants.h"
//#include "../Qt/DockSetSwitchIndex/SetSwitchIndexHelper.h"

namespace DAVA
{
class CommandToggleHangingObjects: public Command
{
public:
	CommandToggleHangingObjects(float value,bool isEnabled);

protected:
	float	value;
	bool	isEnabled;
	
    virtual void Execute();
};

};
#endif // #ifndef __RESOURCE_EDITOR_HANGING_OBJECTS_COMMANDS_H__