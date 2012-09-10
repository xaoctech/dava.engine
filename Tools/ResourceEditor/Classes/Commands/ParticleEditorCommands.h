#ifndef __PARTICLE_EDITOR_COMMANDS_H__
#define __PARTICLE_EDITOR_COMMANDS_H__

#include "Command.h"

class CommandOpenParticleEditorConfig: public Command
{
public:	
	CommandOpenParticleEditorConfig();

protected:	

	virtual void Execute();
};

class CommandSaveParticleEditorConfig: public Command
{
public:	
	CommandSaveParticleEditorConfig();

protected:	

	virtual void Execute();
};

#endif //__PARTICLE_EDITOR_COMMANDS_H__