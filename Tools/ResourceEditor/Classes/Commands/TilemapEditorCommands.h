#ifndef __RESOURCEEDITORQT__TILEMAPEDITORCOMMANDS__
#define __RESOURCEEDITORQT__TILEMAPEDITORCOMMANDS__

#include "DAVAEngine.h"
#include "Command.h"
#include "../SceneEditor/LandscapeEditorColor.h"

class CommandDrawTilemap: public Command
{
public:
	DAVA_DEPRECATED(CommandDrawTilemap(Image* originalImage, Image* newImage, const FilePath & pathname, Landscape* landscape));// DEPRECATED: using of SceneEditorScreenMain, editorBodyControl
	virtual ~CommandDrawTilemap();

protected:
	Image* undoImage;
	Image* redoImage;
	FilePath savedPathname;

	Landscape* landscape;

	virtual void Execute();
	virtual void Cancel();

	LandscapeEditorColor* GetEditor();
	LandscapeEditorBase* GetActiveEditor();
	void UpdateLandscapeTilemap(Image* image);
};

#endif /* defined(__RESOURCEEDITORQT__TILEMAPEDITORCOMMANDS__) */
