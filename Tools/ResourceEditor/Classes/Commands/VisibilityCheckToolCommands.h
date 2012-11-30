#ifndef __RESOURCE_EDITOR_VISIBILITY_CHECK_TOOL_COMMANDS_H__
#define __RESOURCE_EDITOR_VISIBILITY_CHECK_TOOL_COMMANDS_H__

#include "Command.h"
#include "DAVAEngine.h"

using namespace DAVA;

class CommandToggleVisibilityTool: public Command
{
public:
	CommandToggleVisibilityTool();
	
protected:
	virtual void Execute();
};

class CommandSetAreaVisibilityTool: public Command
{
public:
	CommandSetAreaVisibilityTool();
	
protected:
	virtual void Execute();
};

class CommandSetPointVisibilityTool: public Command
{
public:
	CommandSetPointVisibilityTool();
	
protected:
	virtual void Execute();
};

class CommandSaveTextureVisibilityTool: public Command
{
public:
	CommandSaveTextureVisibilityTool();
	
protected:
	virtual void Execute();
};

class CommandChangeAreaSizeVisibilityTool: public Command
{
public:
	CommandChangeAreaSizeVisibilityTool(uint32 newSize);
	
protected:
	uint32 size;
	
	virtual void Execute();
};

#endif