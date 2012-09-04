#ifndef __PARTICLE_EDITOR_COMMANDS_H__
#define __PARTICLE_EDITOR_COMMANDS_H__

#include "Command.h"

#endif //__PARTICLE_EDITOR_COMMANDS_H__


class CommandOpenParticleEditorConfig: public Command
{
public:	
	CommandOpenParticleEditorConfig();

protected:	

	virtual void Execute();
};