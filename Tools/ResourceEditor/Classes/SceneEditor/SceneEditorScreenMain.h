#ifndef __SCENE_EDITOR_SCREEN_MAIN_H__
#define __SCENE_EDITOR_SCREEN_MAIN_H__

#include "DAVAEngine.h"
#include "LibraryControl.h"
#include "MenuPopupControl.h"

//#include "CreateNodeDialog.h"
#include "CreateNodesDialog.h"

#include "SceneNodeIDs.h"

using namespace DAVA;

class LandscapeEditorControl;
class EditorBodyControl;
class MaterialEditor;
class SceneEditorScreenMain: 
    public UIScreen, public UIFileSystemDialogDelegate, public LibraryControlDelegate, 
//    public MenuPopupDelegate, public CreateNodeDialogDelegeate
public MenuPopupDelegate, public CreateNodesDialogDelegeate
{
	struct BodyItem;

    enum eConst
    {        
        MENU_HEIGHT = 20,
        LINE_HEIGHT = 1,

        BODY_Y_OFFSET = 50,
        
        LIBRARY_WIDTH = 200,
        
        TAB_BUTTONS_OFFSET = 200,
    };

    enum DIALOG_OPERATION
    {
        DIALOG_OPERATION_NONE = 0,
        DIALOG_OPERATION_MENU_OPEN,
        DIALOG_OPERATION_MENU_SAVE,
        DIALOG_OPERATION_MENU_PROJECT,
        DIALOG_OPERATION_MENU_EXPORT,
    };
    
    enum eMenuIDS
    {
        MENUID_CREATENODE = 100,
        MENUID_NEW = 200,
    };
    
    enum eNewMenuIDS
    {
        ENMID_ENPTYSCENE = 0,
        ENMID_SCENE_WITH_CAMERA,
        
        ENMID_COUNT
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

    //menu
    virtual void MenuCanceled();
	virtual void MenuSelected(int32 menuID, int32 itemID);
    virtual WideString MenuItemText(int32 menuID, int32 itemID);
    virtual int32 MenuItemsCount(int32 menuID);

    // create node dialog
    virtual void DialogClosed(int32 retCode);

    void ShowMaterialEditor();

	BodyItem * FindCurrentBody();
    
private:
    
    void AddLineControl(Rect r);
    
    //menu
    void CreateTopMenu();
    void ReleaseTopMenu();

    UIButton * btnOpen;
    UIButton * btnSave;
    UIButton * btnExport;
    UIButton * btnMaterials;
    UIButton * btnCreate;
    UIButton * btnNew;
    UIButton * btnProject;
	UIButton * btnBeast;
	UIButton * btnLandscape;
    
    void OnOpenPressed(BaseObject * obj, void *, void *);
    void OnSavePressed(BaseObject * obj, void *, void *);
    void OnExportPressed(BaseObject * obj, void *, void *);
    void OnMaterialsPressed(BaseObject * obj, void *, void *);
    void OnCreatePressed(BaseObject * obj, void *, void *);
    void OnNewPressed(BaseObject * obj, void *, void *);
    void OnOpenProjectPressed(BaseObject * obj, void *, void *);
	void OnBeastPressed(BaseObject * obj, void *, void *);
	void OnLandscapePressed(BaseObject * obj, void *, void *);
        
    //Body list
    struct BodyItem
    {
        UIButton *headerButton;
        UIButton *closeButton;
        EditorBodyControl *bodyControl;
    };
    
    
    Vector<BodyItem *> bodies;
    
    void InitializeBodyList();
    void ReleaseBodyList();
    void AddBodyItem(const WideString &text, bool isCloseable);
    
    void OnSelectBody(BaseObject * owner, void * userData, void * callerData);
    void OnCloseBody(BaseObject * owner, void * userData, void * callerData);
    
    //FileDialog
    UIFileSystemDialog * fileSystemDialog;
    uint32 fileSystemDialogOpMode;
    
    void OnFileSelected(UIFileSystemDialog *forDialog, const String &pathToFile);
    void OnFileSytemDialogCanceled(UIFileSystemDialog *forDialog);

    //SceneGraph
    UIButton *sceneGraphButton;
    void OnSceneGraphPressed(BaseObject * obj, void *, void *);

    //DataGraph
    UIButton *dataGraphButton;
    void OnDataGraphPressed(BaseObject * obj, void *, void *);

    
    //Library
    UIButton *libraryButton;
    LibraryControl *libraryControl;
    void OnLibraryPressed(BaseObject * obj, void *, void *);

    UIButton *propertiesButton;
    void OnPropertiesPressed(BaseObject * obj, void *, void *);
    

    // menu
    MenuPopupControl *menuPopup;

    //create node dialog
//    CreateNodeDialog *nodeDialog;
    CreateNodesDialog *nodeDialog;
    
    MaterialEditor *materialEditor;
//    CreateNodeDialog *nodeDialogs[ECNID_COUNT];
//    int32 currentNodeDialog;
    void InitializeNodeDialogs();
    void ReleaseNodeDialogs();
    
    UIControl *dialogBack;
    
    //Landscape
    LandscapeEditorControl *landscapeEditor;
    
    // general
    Font *font;
    KeyedArchive *keyedArchieve;

};

#endif // __SCENE_EDITOR_SCREEN_MAIN_H__