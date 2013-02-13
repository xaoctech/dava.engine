#ifndef __RESOURCEEDITORQT__EDITCOMMANDS__
#define __RESOURCEEDITORQT__EDITCOMMANDS__

#include "DAVAEngine.h"
#include "Command.h"

class UndoCommand: public Command
{
public:
	UndoCommand();

protected:
	virtual void Execute();
};

class RedoCommand: public Command
{
public:
	RedoCommand();

protected:
	virtual void Execute();
};

#endif /* defined(__RESOURCEEDITORQT__EDITCOMMANDS__) */
