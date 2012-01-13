#ifndef __LIBRARY_CONTROL_H__
#define __LIBRARY_CONTROL_H__

#include "DAVAEngine.h"
#include "../UIFileTree.h"
//#include "CameraController.h"
#include "../EditorScene.h"

using namespace DAVA;

class LibraryControlDelegate
{
public:
	virtual void OnEditSCE(const String &pathName, const String &name) = 0;
	virtual void OnAddSCE(const String &pathName) = 0;
};

class ScenePreviewControl;
class LibraryControl : public UIControl, public UIFileTreeDelegate
{
    enum eConst
    {
        CELL_HEIGHT = 20,
        BUTTON_HEIGHT = 20,
    };
    
public:
    LibraryControl(const Rect & rect);
    virtual ~LibraryControl();
    
    virtual void WillAppear();
	virtual void Update(float32 timeElapsed);

    void SetPath(const String &path);
    
    void SetDelegate(LibraryControlDelegate *delegate);
    
    
    virtual int32 CellHeight(UIList *forList, int32 index);
    virtual void OnCellSelected(UIFileTree * tree, UIFileTreeCell *selectedCell);

protected:

    void RefreshTree();
    
    UIFileTree *fileTreeControl;

    
    UIControl *panelDAE;
    UIButton *btnConvert;
    void OnConvertPressed(BaseObject * object, void * userData, void * callerData);
    
    UIControl *panelSCE;
    UIButton *btnEdit;
    UIButton *btnAdd;
    ScenePreviewControl *preview;
    void OnAddPressed(BaseObject * object, void * userData, void * callerData);
    void OnEditPressed(BaseObject * object, void * userData, void * callerData);
    
    // general
    Font *fontLight;
    Font *fontDark;
    
    UIButton *refreshButton;
    void OnRefreshPressed(BaseObject * object, void * userData, void * callerData);
    
    String folderPath;
    String selectedFileName;
    String selectedFileNameShort;
    
    LibraryControlDelegate *controlDelegate;
    
    UIStaticText *errorMessage;
};



#endif // __LIBRARY_CONTROL_H__