#ifndef __RESOURCEEDITORQT__TILEMAPEDITORCOMMANDS__
#define __RESOURCEEDITORQT__TILEMAPEDITORCOMMANDS__

#include "DAVAEngine.h"
#include "Command.h"
#include "../SceneEditor/LandscapeEditorColor.h"

class CommandTilemapEditor: public Command
{
public:
	CommandTilemapEditor();
    
protected:
    
    virtual void Execute();
};

class CommandDrawTilemap: public Command
{
public:
	CommandDrawTilemap();
	virtual ~CommandDrawTilemap();

protected:
	Image* undoImage;
	Image* redoImage;

	virtual void Execute();
	virtual void Cancel();

	LandscapeEditorColor* GetEditor();
};

#endif /* defined(__RESOURCEEDITORQT__TILEMAPEDITORCOMMANDS__) */
