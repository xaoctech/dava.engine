#ifndef __LANDSCAPE_EDITOR_BASE_H__
#define __LANDSCAPE_EDITOR_BASE_H__

#include "DAVAEngine.h"
#include "LandscapeToolsPanel.h"

using namespace DAVA;


class LandscapeEditorDelegate
{
public:
    
    virtual void LandscapeEditorStarted() = 0;  //Show LE Controls
    virtual void LandscapeEditorFinished() = 0; //Hide LE Controls
};

class EditorBodyControl;
class EditorScene;
class LandscapeTool;
class HeightmapNode;
class NodesPropertyControl;
class LandscapeEditorBase
    :   public BaseObject
    ,   public UIFileSystemDialogDelegate
    ,   public LandscapeToolsPanelDelegate
{
    enum eLEState
    {
        ELE_NONE = -1,
        ELE_ACTIVE,
        ELE_CLOSING,
        ELE_SAVING_TEXTURE,
        ELE_TEXTURE_SAVED
    };
    
    enum DIALOG_OPERATION
    {
        DIALOG_OPERATION_NONE = -1,
        DIALOG_OPERATION_SAVE,
    };

    static const int32 INVALID_TOUCH_ID = -1;
    static const int32 RAY_TRACING_DISTANCE = 1000;
    
    
public:
    
    LandscapeEditorBase(LandscapeEditorDelegate *newDelegate, EditorBodyControl *parentControl); 
    virtual ~LandscapeEditorBase();
    
	virtual void Draw(const UIGeometricData &geometricData);
    virtual void Update(float32 timeElapsed);
    bool Input(UIEvent * touch);

    virtual bool SetScene(EditorScene *newScene);
    void SetTool(LandscapeTool *newTool);

    void Toggle();
    bool IsActive();
    
    LandscapeNode *GetLandscape();
    
    LandscapeToolsPanel *GetToolPanel();
    virtual NodesPropertyControl *GetPropertyControl(const Rect &rect) = 0;
    
    //file dialog delegate
    virtual void OnFileSelected(UIFileSystemDialog *forDialog, const String &pathToFile);
    virtual void OnFileSytemDialogCanceled(UIFileSystemDialog *forDialog);

    //Tools Panel delegate
    virtual void OnToolSelected(LandscapeTool *newTool);
    virtual void OnShowGrid(bool show);

	virtual void ClearSceneResources();

	virtual void UpdateLandscapeTilemap(Texture* texture) {};

protected:

    virtual void SaveTexture();
    void SaveTextureAs(const String &pathToFile, bool closeLE);

    virtual void InputAction(int32 phase, bool intersects) = 0;
    virtual void HideAction() = 0;
    virtual void ShowAction() = 0;
    virtual void SaveTextureAction(const String &pathToFile) = 0;
	virtual void UpdateCursor() = 0;
    
    virtual void RecreateHeightmapNode() = 0;
    
    void Close();
    LandscapeEditorDelegate *delegate;

    bool GetLandscapePoint(const Vector2 &touchPoint, Vector2 &landscapePoint);
    
    UIFileSystemDialog * fileSystemDialog;
    uint32 fileSystemDialogOpMode;
    
    eLEState state;

    HeightmapNode *heightmapNode;
    LandscapeNode *workingLandscape;

    LandscapeTool *currentTool;

    String savedPath;
    int32 landscapeSize;

    EditorScene *workingScene;    
    EditorBodyControl *parent;

    Vector2 landscapePoint;
    Vector2 prevDrawPos;
    
    LandscapeToolsPanel *toolsPanel;

    bool inverseDrawingEnabled;
    int32 touchID;
    
	Texture * cursorTexture;

    
    LandscapeNode::eTiledShaderMode savedShaderMode;
};


#endif //__LANDSCAPE_EDITOR_BASE_H__
