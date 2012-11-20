#ifndef __LANDSCAPE_EDITOR_CUSTOM_COLOR_H__
#define __LANDSCAPE_EDITOR_CUSTOM_COLOR_H__

#include "DAVAEngine.h"
#include "LandscapeEditorBase.h"
#include "LandscapeToolsPanel.h"
#include "LandscapeEditorPropertyControl.h"

using namespace DAVA;

class EditorHeightmap;
class LandscapeEditorCustomColors
    :   public LandscapeEditorBase
    ,   public LandscapeEditorPropertyControlDelegate

{
    
public:
    
    LandscapeEditorCustomColors(LandscapeEditorDelegate *newDelegate, EditorBodyControl *parentControl, const Rect &toolsRect);
    virtual ~LandscapeEditorCustomColors();
    
	virtual void Draw(const UIGeometricData &geometricData);

    virtual NodesPropertyControl *GetPropertyControl(const Rect &rect);
    
    virtual bool SetScene(EditorScene *newScene);
	virtual void SaveTexture()
	{ Close(); };
    
    //LE property control delegate
    virtual void LandscapeEditorSettingsChanged(LandscapeEditorSettings *settings);
    virtual void TextureWillChanged(const String &forKey);
    virtual void TextureDidChanged(const String &forKey);
    

	void SetColor(const Color &newColor);
	void SetRadius(int radius);
	void SaveColorLayer(const String &pathName);
	void UpdateTool();
	

protected:

    virtual void InputAction(int32 phase, bool intersects);
    virtual void HideAction();
    virtual void ShowAction();
    virtual void SaveTextureAction(const String &pathToFile);
	virtual void UpdateCursor();
    virtual void UndoAction();
    virtual void RedoAction();
	
    virtual void RecreateHeightmapNode();
	void UpdateCircleTexture();


	void PrepareRenderLayers();

    void PerformLandscapeDraw();
    void DrawCircle(Vector<Vector<bool> >& matrixForCircle) ;
    uint8*	DrawFilledCircleWithFormat(uint32 radius, DAVA::PixelFormat format);

    EditorHeightmap *editedHeightmap;
    Heightmap *savedHeightmap;


	bool wasTileMaskToolUpdate;
    
    LandscapeEditorSettings *settings;

    Color paintColor;

    bool editingIsEnabled;

	Texture*	circleTexture;
	Sprite*		colorSprite;
	Texture*	texSurf;

	int32		radius;

	Sprite*		currentToolSprite;
};


#endif //__LANDSCAPE_EDITOR_CUSTOM_COLOR_H__
