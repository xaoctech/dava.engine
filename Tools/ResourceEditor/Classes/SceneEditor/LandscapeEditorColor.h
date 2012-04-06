#ifndef __LANDSCAPE_EDITOR_H__
#define __LANDSCAPE_EDITOR_H__

#include "DAVAEngine.h"
#include "LandscapeToolsPanel.h"
#include "LandscapeEditorPropertyControl.h"

using namespace DAVA;

class LandscapeEditorDelegate
{
public:
    
    virtual void LandscapeEditorStarted() = 0;  //Show LE Controls
    virtual void LandscapeEditorFinished() = 0; //Hide LE Controls
};


class PaintTool;
class HeightmapNode;
class EditorScene;
class EditorBodyControl;

class LandscapeEditor
    :   public BaseObject
    ,   public UIFileSystemDialogDelegate
    ,   public LandscapeToolsPanelDelegate
    ,   public LandscapeEditorPropertyControlDelegate

{
    enum eLEState
    {
        ELE_NONE = -1,
        ELE_ACTIVE,
        ELE_CLOSING,
        ELE_SAVING_MASK,
        ELE_MASK_SAVED
    };

    enum DIALOG_OPERATION
    {
        DIALOG_OPERATION_NONE = -1,
        DIALOG_OPERATION_SAVE,
    };
    
    enum eLEConst
    {
        ZOOM_MULTIPLIER = 4
    };
    
public:
    
    LandscapeEditor(LandscapeEditorDelegate *newDelegate, EditorBodyControl *parentControl); 
    virtual ~LandscapeEditor();
    
	void Draw(const UIGeometricData &geometricData);
    bool Input(UIEvent * touch);

    bool SetScene(EditorScene *newScene);
    void SetPaintTool(PaintTool *newTool);
    void SetSettings(LandscapeEditorSettings *newSettings);
    void Toggle();
    bool IsActive();
    
    LandscapeNode *GetLandscape();
    
    //Tools Panel delegate
    virtual void OnToolSelected(PaintTool *newTool);
    
    //LE property control delegate
    virtual void LandscapeEditorSettingsChanged(LandscapeEditorSettings *settings);
    virtual void MaskTextureWillChanged();
    virtual void MaskTextureDidChanged();
        
    //file dialog delegate
    virtual void OnFileSelected(UIFileSystemDialog *forDialog, const String &pathToFile);
    virtual void OnFileSytemDialogCanceled(UIFileSystemDialog *forDialog);

protected:

    void SaveNewMask();
    void SaveMaskAs(const String &pathToFile, bool closeLE);
    void CreateMaskTexture();

    bool GetLandscapePoint(const Vector2 &touchPoint, Vector2 &landscapePoint);
	void UpdateTileMaskTool();
    void UpdateTileMask();
    
    void Close();
    
    
    LandscapeEditorDelegate *delegate;

    eLEState state;
    
    UIFileSystemDialog * fileSystemDialog;
    uint32 fileSystemDialogOpMode;
    
    bool savedModificatioMode;

    HeightmapNode *heightmapNode;
    LandscapeNode *workingLandscape;

    Texture *savedTexture;
    Sprite *maskSprite;
	Sprite *oldMaskSprite;
	Sprite *toolSprite;
    
	bool wasTileMaskToolUpdate;
    
    LandscapeEditorSettings *settings;
    PaintTool *currentTool;
    
    eBlendMode srcBlendMode;
    eBlendMode dstBlendMode;
    Color paintColor;
    Vector2 startPoint;
    Vector2 endPoint;
    Vector2 prevDrawPos;
    
    bool isPaintActive;
    
	Shader * tileMaskEditorShader;
    
    EditorScene *workingScene;
    
    EditorBodyControl *parent;
};


#endif //__LANDSCAPE_EDITOR_H__
