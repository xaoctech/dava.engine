#ifndef __LANDSCAPE_EDITOR_HEIGHTMAP_H__
#define __LANDSCAPE_EDITOR_HEIGHTMAP_H__

#include "DAVAEngine.h"
#include "LandscapeEditorBase.h"
#include "LandscapeToolsPanel.h"
#include "Scene3D/LandscapeDebugNode.h"

using namespace DAVA;

class EditorScene;
class LandscapeEditorHeightmap
    :   public LandscapeEditorBase
{
    enum eLEConst
    {
        ZOOM_MULTIPLIER = 4
    };
    
public:
    
    LandscapeEditorHeightmap(LandscapeEditorDelegate *newDelegate, EditorBodyControl *parentControl, const Rect &toolsRect); 
    virtual ~LandscapeEditorHeightmap();
    
    virtual void Draw(const UIGeometricData &geometricData);

    virtual NodesPropertyControl *GetPropertyControl(const Rect &rect);
    
protected:

    virtual void InputAction();
    virtual void HideAction();
    virtual void ShowAction();
    virtual void SaveTextureAction(const String &pathToFile);

    
    void CreateMaskTexture();

	void UpdateTileMaskTool();
    
	bool wasTileMaskToolUpdate;

    LandscapeDebugNode *landscapeDebugNode;
    Image *heightImage;
};


#endif //__LANDSCAPE_EDITOR_HEIGHTMAP_H__
