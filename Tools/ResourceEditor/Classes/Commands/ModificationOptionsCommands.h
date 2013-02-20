#ifndef __RESOURCEEDITORQT__MODIFICATIONOPTIONSCOMMANDS__
#define __RESOURCEEDITORQT__MODIFICATIONOPTIONSCOMMANDS__

#include "Command.h"
#include "../Constants.h"
#include "DAVAEngine.h"

using DAVA::float32;

class ModificationPlaceOnLandCommand: public Command
{
public:
	ModificationPlaceOnLandCommand();

protected:
	virtual void Execute();
};

class ModificationApplyCommand: public Command
{
public:
	ModificationApplyCommand(float32 x, float32 y, float32 z);

protected:
	virtual void Execute();

private:
	float32 x;
	float32 y;
	float32 z;
};

class ModificationResetCommand: public Command
{
public:
	ModificationResetCommand();

protected:
	virtual void Execute();
};

#endif /* defined(__RESOURCEEDITORQT__MODIFICATIONOPTIONSCOMMANDS__) */
