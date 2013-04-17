#ifndef __TOOLS_COMMANDS_H__
#define __TOOLS_COMMANDS_H__

#include "Command.h"
#include "../Constants.h"

class CommandMaterials: public Command
{
public:	
	CommandMaterials();

protected:	
    
    virtual void Execute();
};


class CommandTextureConverter: public Command
{
public:	
	CommandTextureConverter();
    
protected:	
    
    virtual void Execute();
};


class CommandSettings: public Command
{
public:
	CommandSettings();
    
protected:
    
    virtual void Execute();
};


class CommandBeast: public Command
{
public:
	CommandBeast();
    
protected:
    
    virtual void Execute();
};

class CommandRulerTool: public Command
{
public:
	CommandRulerTool();
    
protected:
    
    virtual void Execute();
};

class CommandConvertToShadow : public Command
{
public:
	CommandConvertToShadow();

protected:
	virtual void Execute();
};


#endif // #ifndef __TOOLS_COMMANDS_H__