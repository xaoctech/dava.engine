#ifndef __RESOURCEEDITORQT__HEIGHTMAPEDITORCOMMANDS__
#define __RESOURCEEDITORQT__HEIGHTMAPEDITORCOMMANDS__

#include "DAVAEngine.h"
#include "Command.h"
#include "../SceneEditor/LandscapeEditorHeightmap.h"

class HeightmapModificationCommand: public Command
{
public:
	HeightmapModificationCommand(Command::eCommandType type,
								 const Rect& updatedRect,
								 CommandList::eCommandId id);

protected:
	Rect updatedRect;

	static String TimeString();
	static FilePath SaveHeightmap(Heightmap* heightmap);
	static LandscapeEditorHeightmap* GetEditor();
	static void UpdateLandscapeHeightmap(const FilePath & filename);
};

class CommandDrawHeightmap: public HeightmapModificationCommand
{
public:
	DAVA_DEPRECATED(CommandDrawHeightmap(Heightmap* originalHeightmap, // DEPRECATED : using SceneDataManager(QOBJECT)
						 Heightmap* newHeightmap,
						 const Rect& updatedRect));
	virtual ~CommandDrawHeightmap();
	
protected:
	FilePath undoFilename;
	FilePath redoFilename;

	virtual void Execute();
	virtual void Cancel();
};

class CommandCopyPasteHeightmap: public HeightmapModificationCommand
{
public:
	DAVA_DEPRECATED(CommandCopyPasteHeightmap(bool copyHeightmap, bool copyTilemap, //DEPRECATED: using of SceneEditorScreenMain, SceneDataManager...
							  Heightmap* originalHeightmap, Heightmap* newHeightmap,
							  Image* originalTilemap, Image* newTilemap,
							  const FilePath& tilemapSavedPath, const Rect& updatedRect));
	virtual ~CommandCopyPasteHeightmap();

protected:
	FilePath heightmapUndoFilename;
	FilePath heightmapRedoFilename;

	Image* tilemapUndoImage;
	Image* tilemapRedoImage;
	FilePath tilemapSavedPathname;

	Landscape* landscape;

	bool heightmap;
	bool tilemap;

	virtual void Execute();
	virtual void Cancel();

	void UpdateLandscapeTilemap(Image* image);
	LandscapeEditorBase* GetActiveEditor();
};

#endif /* defined(__RESOURCEEDITORQT__HEIGHTMAPEDITORCOMMANDS__) */
