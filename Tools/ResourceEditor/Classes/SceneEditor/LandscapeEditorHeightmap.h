#ifndef __LANDSCAPE_EDITOR_HEIGHTMAP_H__
#define __LANDSCAPE_EDITOR_HEIGHTMAP_H__

#include "DAVAEngine.h"
#include "LandscapeEditorBase.h"
#include "LandscapeToolsPanel.h"
#include "Scene3D/LandscapeDebugNode.h"
#include "Scene3D/Heightmap.h"

using namespace DAVA;

class EditorScene;
class LandscapeEditorHeightmap
    :   public LandscapeEditorBase
{
    
public:
    
    LandscapeEditorHeightmap(LandscapeEditorDelegate *newDelegate, EditorBodyControl *parentControl, const Rect &toolsRect); 
    virtual ~LandscapeEditorHeightmap();
    
    virtual void Update(float32 timeElapsed);

    virtual NodesPropertyControl *GetPropertyControl(const Rect &rect);
    
    //Tools Panel delegate
    virtual void OnToolSelected(LandscapeTool *newTool);

protected:

    virtual void InputAction(int32 phase);
    virtual void HideAction();
    virtual void ShowAction();
    virtual void SaveTextureAction(const String &pathToFile);
	virtual void UpdateCursor();

    
	void UpdateTileMaskTool(float32 timeElapsed);
    void UpdateToolImage();
    
    bool editingIsEnabled;
    
	bool wasTileMaskToolUpdate;

    LandscapeDebugNode *landscapeDebugNode;
    Heightmap *heightmap;
    
    Image *toolImage;
};


#endif //__LANDSCAPE_EDITOR_HEIGHTMAP_H__
