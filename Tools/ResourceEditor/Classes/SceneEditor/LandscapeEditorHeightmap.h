#ifndef __LANDSCAPE_EDITOR_HEIGHTMAP_H__
#define __LANDSCAPE_EDITOR_HEIGHTMAP_H__

#include "DAVAEngine.h"
#include "LandscapeEditorBase.h"
#include "LandscapeToolsPanel.h"
#include "Scene3D/LandscapeDebugNode.h"
#include "Scene3D/Heightmap.h"
#include "LandscapeEditorPropertyControl.h"

using namespace DAVA;

class EditorScene;
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
    virtual void TextureWillChanged();
    virtual void TextureDidChanged();

    
protected:

    virtual void InputAction(int32 phase, bool intersects);
    virtual void HideAction();
    virtual void ShowAction();
    virtual void SaveTextureAction(const String &pathToFile);
	virtual void UpdateCursor();
    virtual void UndoAction();
    virtual void RedoAction();

    
	void UpdateTileMaskTool(float32 timeElapsed);
    void UpdateToolImage();
    float32 GetDropperHeight();
    
    bool editingIsEnabled;
    
	bool wasTileMaskToolUpdate;

    LandscapeDebugNode *landscapeDebugNode;
    Heightmap *heightmap;
    
    Image *toolImage;
    float32 prevToolSize;
};


#endif //__LANDSCAPE_EDITOR_HEIGHTMAP_H__
