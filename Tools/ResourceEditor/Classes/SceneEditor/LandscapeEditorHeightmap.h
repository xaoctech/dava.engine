#ifndef __LANDSCAPE_EDITOR_HEIGHTMAP_H__
#define __LANDSCAPE_EDITOR_HEIGHTMAP_H__

#include "DAVAEngine.h"
#include "LandscapeEditorBase.h"
#include "LandscapeToolsPanel.h"
#include "LandscapeEditorPropertyControl.h"

using namespace DAVA;

class EditorScene;
class EditorHeightmap;
class EditorLandscape;
class LandscapesController;
class CommandDrawHeightmap;
class LandscapeEditorHeightmap
    :   public LandscapeEditorBase
    ,   public LandscapeEditorPropertyControlDelegate
{
    
public:
    
    LandscapeEditorHeightmap(LandscapeEditorDelegate *newDelegate, EditorBodyControl *parentControl, const Rect &toolsRect); 
    virtual ~LandscapeEditorHeightmap();
    
    virtual void Update(float32 timeElapsed);

    virtual NodesPropertyControl *GetPropertyControl(const Rect &rect);
    
    //Tools Panel delegate
    virtual void OnToolSelected(LandscapeTool *newTool);
    virtual void OnShowGrid(bool show);

    //LE property control delegate
    virtual void LandscapeEditorSettingsChanged(LandscapeEditorSettings *settings);
    virtual void TextureWillChanged(const String &forKey);
    virtual void TextureDidChanged(const String &forKey);

	Heightmap* GetHeightmap();
	void UpdateHeightmap(Heightmap* heightmap, Rect rect = Rect(-1, -1, -1, -1));

	virtual void UpdateLandscapeTilemap(Texture* texture);
protected:
	void StoreOriginalHeightmap();
	void CreateUndoPoint();
	void CreateHeightmapUndo();
	void CreateCopyPasteUndo();

    virtual void InputAction(int32 phase, bool intersects);
    virtual void HideAction();
    virtual void ShowAction();
    virtual void SaveTextureAction(const FilePath &pathToFile);
	virtual void UpdateCursor();

    bool CopyPasteBegin();
    virtual void RecreateHeightmapNode();

    
	void UpdateTileMaskTool(float32 timeElapsed);
	void UpdateBrushTool(float32 timeElapsed);
	void UpdateCopypasteTool(float32 timeElapsed);

    
    void UpdateToolImage();
    float32 GetDropperHeight();
    
    void UpdateHeightmap(const Rect &updatedRect);
    
    
    bool editingIsEnabled;
    
	bool wasTileMaskToolUpdate;

    LandscapesController *landscapesController;
    
    Image *toolImage;
    Image *toolImageTile;
    float32 prevToolSize;
    
    Vector2 copyFromCenter;
    Vector2 copyToCenter;
    Image *tilemaskImage;
    FilePath tilemaskPathname;
    bool tilemaskWasChanged;
    Texture *tilemaskTexture;
    
    void CreateTilemaskImage();
    Image *CreateToolImage(int32 sideSize);

	Heightmap* oldHeightmap;
	Image* oldTilemap;
};


#endif //__LANDSCAPE_EDITOR_HEIGHTMAP_H__
