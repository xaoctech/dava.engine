#ifndef __SCENE_EDITOR_SCREEN_MAIN_H__
#define __SCENE_EDITOR_SCREEN_MAIN_H__

#include "DAVAEngine.h"
#include "LibraryControl.h"

using namespace DAVA;

class EditorBodyControl;
class SceneEditorScreenMain: public UIScreen, public UIFileSystemDialogDelegate, public LibraryControlDelegate
{
    enum eConst
    {
        BUTTON_HEIGHT = 20,
        BUTTON_WIDTH = 100, 
        
        MENU_HEIGHT = 20,
        LINE_HEIGHT = 1,

        BODY_Y_OFFSET = 50,
        
        LIBRARY_WIDTH = 200,
    };
    
public:

	virtual void LoadResources();
	virtual void UnloadResources();
	virtual void WillAppear();
	virtual void WillDisappear();
	
	virtual void Update(float32 timeElapsed);
	virtual void Draw(const UIGeometricData &geometricData);

	virtual void OnEditSCE(const String &pathName, const String &name);
	virtual void OnAddSCE(const String &pathName);
    
private:
    
    int32 FindCurrentBody();
    
    
    void AddLineControl(Rect r);
    
    void CustomizeButton(UIButton *btn, const WideString &text);
    
    //menu
    void CreateTopMenu();
    void ReleaseTopMenu();
    UIButton *CustomiseMenuButton(Rect r, const WideString &text);
    
    UIButton * btnOpen;
    UIButton * btnSave;
    UIButton * btnMaterials;
    UIButton * btnCreate;
    UIButton * btnNew;
    UIButton * btnProject;
    
    void OnOpenPressed(BaseObject * obj, void *, void *);
    void OnSavePressed(BaseObject * obj, void *, void *);
    void OnMaterialsPressed(BaseObject * obj, void *, void *);
    void OnCreatePressed(BaseObject * obj, void *, void *);
    void OnNewPressed(BaseObject * obj, void *, void *);
    void OnOpenProjectPressed(BaseObject * obj, void *, void *);
        
    //Body list
    struct BodyItem
    {
        UIButton *headerButton;
        UIButton *closeButton;
        EditorBodyControl *bodyControl;
    };
    
    Vector<BodyItem> bodies;
    
    void InitializeBodyList();
    void ReleaseBodyList();
    void AddBodyItem(const WideString &text, bool isCloseable);
    
    void OnSelectBody(BaseObject * owner, void * userData, void * callerData);
    void OnCloseBody(BaseObject * owner, void * userData, void * callerData);
    
    //FileDialog
    UIFileSystemDialog * fileSystemDialog;
    enum DIALOG_OPERATION
    {
        DIALOG_OPERATION_NONE = 0,
        DIALOG_OPERATION_MENU_OPEN,
        DIALOG_OPERATION_MENU_SAVE,
        DIALOG_OPERATION_MENU_PROJECT,
    };
    uint32 fileSystemDialogOpMode;
    
    void OnFileSelected(UIFileSystemDialog *forDialog, const String &pathToFile);
    void OnFileSytemDialogCanceled(UIFileSystemDialog *forDialog);

    //Library
    UIButton *libraryButton;
    LibraryControl *libraryControl;
    void OnLibraryPressed(BaseObject * obj, void *, void *);

    UIButton *propertiesButton;
    void OnPropertiesPressed(BaseObject * obj, void *, void *);
    
    
    // general
    Font *font;
    KeyedArchive *keyedArchieve;

};

#endif // __SCENE_EDITOR_SCREEN_MAIN_H__