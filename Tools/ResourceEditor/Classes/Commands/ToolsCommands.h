#ifndef __TOOLS_COMMANDS_H__
#define __TOOLS_COMMANDS_H__

#include "Command.h"
#include "../Constants.h"

class CommandBeast: public Command
{
public:
	DAVA_DEPRECATED(CommandBeast());
    
protected:
    
    virtual void Execute();
};

class CommandConvertToShadow : public Command
{
public:
	CommandConvertToShadow();

protected:
	virtual void Execute();
	virtual DAVA::Set<DAVA::Entity*> GetAffectedEntities();

	DAVA::Entity* affectedEntity;
};


#endif // #ifndef __TOOLS_COMMANDS_H__