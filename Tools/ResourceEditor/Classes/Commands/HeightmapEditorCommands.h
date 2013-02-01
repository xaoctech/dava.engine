#ifndef __RESOURCEEDITORQT__HEIGHTMAPEDITORCOMMANDS__
#define __RESOURCEEDITORQT__HEIGHTMAPEDITORCOMMANDS__

#include "DAVAEngine.h"
#include "Command.h"
#include "LandscapeEditorHeightmap.h"

class CommandHeightmapEditor: public Command
{
public:
	CommandHeightmapEditor();
    
protected:
    
    virtual void Execute();
};

class CommandDrawHeightmap: public Command
{
public:
	CommandDrawHeightmap();
	virtual ~CommandDrawHeightmap();
	
protected:
	String undoFilename;
	String redoFilename;

	virtual void Execute();
	virtual void Cancel();

	LandscapeEditorHeightmap* GetEditor();

	String TimeString();
	String SaveHeightmap(Heightmap* heightmap, String suffix);
	String GetRandomString(uint32 len);
};

#endif /* defined(__RESOURCEEDITORQT__HEIGHTMAPEDITORCOMMANDS__) */
