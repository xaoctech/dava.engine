#ifndef __LANDSCAPE_EDITOR_CONTROL_H__
#define __LANDSCAPE_EDITOR_CONTROL_H__

#include "DAVAEngine.h"
#include "../UIScrollView.h"
#include "PropertyList.h"

#include "PaintAreaControl.h"

using namespace DAVA;

class PaintAreaControl;
class PaintTool;
class LandscapeEditorControl : public UIControl, public PropertyListDelegate, public UIFileSystemDialogDelegate
{
    enum eConst
    {
        TOOLS_HEIGHT = 40,
        OFFSET = 1,
        TOOL_BUTTON_SIDE = 32,
        SLIDER_WIDTH = 100,
    };
    
    enum DIALOG_OPERATION
    {
        DIALOG_OPERATION_NONE = 0,
        DIALOG_OPERATION_SAVE,
    };

    
public:
    LandscapeEditorControl(const Rect & rect);
    virtual ~LandscapeEditorControl();
    
    virtual void WillAppear();
        
    virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue);
    virtual void OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);

    virtual void OnFileSelected(UIFileSystemDialog *forDialog, const String &pathToFile);
    virtual void OnFileSytemDialogCanceled(UIFileSystemDialog *forDialog);

    
protected:

    void SetDrawingMask(int32 flag, bool value);
    
    void CreateLeftPanel();
    void ReleaseLeftPanel();
    
    void CreateRightPanel();
    void ReleaseRightPanel();

    void CreatePaintAreaPanel();
    void ReleasePaintAreaPanel();
    

    // left side
    UIControl *leftPanel;
    PropertyList *propertyList;

    //right side
    UIControl *rightPanel;

    //paint area
	void OnToolSelected(BaseObject * object, void * userData, void * callerData);

    UIControl *toolsPanel;
    PaintAreaControl *paintArea;
    UIScrollView *scrollView;

    UIControl *toolButtons[PaintTool::EBT_COUNT];
    PaintTool *tools[PaintTool::EBT_COUNT];
    PaintTool *selectedTool;
    
    
    UISlider *radius;
    UISlider *intension;
    UISlider *zoom;
    UISlider * CreateSlider(const Rect & rect);
    void AddSliderHeader(UISlider *slider, const WideString &text);
	void OnRadiusChanged(BaseObject * object, void * userData, void * callerData);
	void OnIntensionChanged(BaseObject * object, void * userData, void * callerData);
	void OnZoomChanged(BaseObject * object, void * userData, void * callerData);
    
    UIFileSystemDialog * fileSystemDialog;
    uint32 fileSystemDialogOpMode;

    
    void OnSavePressed(BaseObject * object, void * userData, void * callerData);

};



#endif // __LANDSCAPE_EDITOR_CONTROL_H__