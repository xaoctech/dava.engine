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

    
public:
    
    LandscapeEditorBase(LandscapeEditorDelegate *newDelegate, EditorBodyControl *parentControl); 
    virtual ~LandscapeEditorBase();
    
	virtual void Draw(const UIGeometricData &geometricData) = 0;
    bool Input(UIEvent * touch);

    virtual bool SetScene(EditorScene *newScene);
    void SetTool(LandscapeTool *newTool);

    void Toggle();
    bool IsActive();
    
    LandscapeNode *GetLandscape();
    
    UIControl *GetToolPanel();
    virtual NodesPropertyControl *GetPropertyControl(const Rect &rect) = 0;
    
    //file dialog delegate
    virtual void OnFileSelected(UIFileSystemDialog *forDialog, const String &pathToFile);
    virtual void OnFileSytemDialogCanceled(UIFileSystemDialog *forDialog);

    //Tools Panel delegate
    virtual void OnToolSelected(LandscapeTool *newTool);
    virtual void OnToolsPanelClose();

protected:

    void SaveTexture();
    void SaveTextureAs(const String &pathToFile, bool closeLE);

    virtual void InputAction() = 0;
    virtual void HideAction() = 0;
    virtual void ShowAction() = 0;
    virtual void SaveTextureAction(const String &pathToFile) = 0;
    
    void Close();
    LandscapeEditorDelegate *delegate;

    bool GetLandscapePoint(const Vector2 &touchPoint, Vector2 &landscapePoint);
    
    UIFileSystemDialog * fileSystemDialog;
    uint32 fileSystemDialogOpMode;
    
    eLEState state;
    bool isPaintActive;

    HeightmapNode *heightmapNode;
    LandscapeNode *workingLandscape;

    LandscapeTool *currentTool;

    Texture *savedTexture;
    int32 landscapeSize;

    EditorScene *workingScene;    
    EditorBodyControl *parent;

    Vector2 startPoint;
    Vector2 endPoint;
    Vector2 prevDrawPos;
    
    LandscapeToolsPanel *toolsPanel;

	Texture * cursorTexture;
};


#endif //__LANDSCAPE_EDITOR_BASE_H__
