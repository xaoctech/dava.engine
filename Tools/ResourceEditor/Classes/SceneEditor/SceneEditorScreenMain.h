#ifndef __SCENE_EDITOR_SCREEN_MAIN_H__
#define __SCENE_EDITOR_SCREEN_MAIN_H__

#include "DAVAEngine.h"
#include "LibraryControl.h"
#include "MenuPopupControl.h"

#include "CreateNodesDialog.h"

#include "SceneNodeIDs.h"
#include "SettingsDialog.h"

using namespace DAVA;

class LandscapeEditorControl;
class EditorBodyControl;
class MaterialEditor;
class SettingsDialog;
class TextureTrianglesDialog;
class SceneEditorScreenMain: 
    public UIScreen, public UIFileSystemDialogDelegate, public LibraryControlDelegate, 
    public MenuPopupDelegate, public CreateNodesDialogDelegeate,
    public SettingsDialogDelegate
{

    enum eConst
    {        
        LINE_HEIGHT = 1,

        BODY_Y_OFFSET = 50,
        
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
        MENUID_OPEN = 100,
        MENUID_CREATENODE = 200,
        MENUID_NEW = 300,
        MENUID_VIEWPORT = 400,
    };
    
    enum eNewMenuIDS
    {
        ENMID_ENPTYSCENE = 0,
        ENMID_SCENE_WITH_CAMERA,
        
        ENMID_COUNT
    };
    
    enum eOpenMenuIDS
    {
        EOMID_OPEN = 0,
        EOMID_OPENLAST_STARTINDEX,
        
        EOMID_COUNT
    };
    

public:
    
    struct BodyItem;


	virtual void LoadResources();
	virtual void UnloadResources();
	virtual void WillAppear();
	virtual void WillDisappear();
	
	virtual void Update(float32 timeElapsed);
	virtual void Draw(const UIGeometricData &geometricData);

	virtual void OnEditSCE(const String &pathName, const String &name);
	virtual void OnAddSCE(const String &pathName);
	virtual void OnReloadSCE(const String &pathName);

    //menu
    virtual void MenuCanceled();
	virtual void MenuSelected(int32 menuID, int32 itemID);
    virtual WideString MenuItemText(int32 menuID, int32 itemID);
    virtual int32 MenuItemsCount(int32 menuID);

    // create node dialog
    virtual void DialogClosed(int32 retCode);

    void EditMaterial(Material *material);
    
    void ShowTextureTriangles(PolygonGroup *polygonGroup);

	BodyItem * FindCurrentBody();
    
    virtual void SettingsChanged();
    virtual void Input(UIEvent * event);

	struct BodyItem
	{
		UIButton *headerButton;
		UIButton *closeButton;
		EditorBodyControl *bodyControl;
	};

private:
    
    void AutoSaveLevel(BaseObject * obj, void *, void *);
    void SetupAnimation();
    
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
	UIButton * btnViewPortSize;

    
    void OnOpenPressed(BaseObject * obj, void *, void *);
    void OnSavePressed(BaseObject * obj, void *, void *);
    void OnExportPressed(BaseObject * obj, void *, void *);
    void OnMaterialsPressed(BaseObject * obj, void *, void *);
    void OnCreatePressed(BaseObject * obj, void *, void *);
    void OnNewPressed(BaseObject * obj, void *, void *);
    void OnOpenProjectPressed(BaseObject * obj, void *, void *);
	void OnBeastPressed(BaseObject * obj, void *, void *);
	void OnLandscapePressed(BaseObject * obj, void *, void *);
    void OnViewPortSize(BaseObject * obj, void *, void *);
    

    //Body list

    
    
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

    UIButton *sceneInfoButton;
    void OnSceneInfoPressed(BaseObject * obj, void *, void *);
    

    void NodeExportPreparation(SceneNode *node);//expand this methods if you need to expand export functionality
    void ExportTexture(const String &textureDataSourcePath);
    
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

    //Open menu
    void ShowOpenFileDialog();
    void ShowOpenLastDialog();
    void OpenFileAtScene(const String &pathToFile);

    //Landscape
    LandscapeEditorControl *landscapeEditor;
    
    void OnSettingsPressed(BaseObject * obj, void *, void *);
    SettingsDialog *settingsDialog;
    
    TextureTrianglesDialog *textureTrianglesDialog;
    
    // general
    Font *font;
};

#endif // __SCENE_EDITOR_SCREEN_MAIN_H__