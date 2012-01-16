#ifndef __LANDSCAPE_EDITOR_CONTROL_H__
#define __LANDSCAPE_EDITOR_CONTROL_H__

#include "DAVAEngine.h"
#include "../UIScrollView.h"
#include "PropertyList.h"

#include "PaintAreaControl.h"

using namespace DAVA;

class PaintAreaControl;
class PaintTool;
class LandscapeEditorControl : public UIControl, public PropertyListDelegate
{
    enum eConst
    {
        TOOLS_HEIGHT = 40,
        OFFSET = 1,
        TOOL_BUTTON_SIDE = 32,
        SLIDER_WIDTH = 100,
    };
    
public:
    LandscapeEditorControl(const Rect & rect);
    virtual ~LandscapeEditorControl();
    
    virtual void WillAppear();
        
    virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue);
    virtual void OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);

    
protected:

    void CreateLeftPanel();
    void ReleaseLeftPanel();
    
    void CreateRightPanel();
    void ReleaseRightPanel();

    void CreatePaintAreaPanel();
    void ReleasePaintAreaPanel();
    
    bool IsValidPath(const String &path);


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
    UISlider *height;
    UISlider *zoom;
    UISlider * CreateSlider(const Rect & rect);
    void AddSliderHeader(UISlider *slider, const WideString &text);
	void OnRadiusChanged(BaseObject * object, void * userData, void * callerData);
	void OnHeightChanged(BaseObject * object, void * userData, void * callerData);
	void OnZoomChanged(BaseObject * object, void * userData, void * callerData);
};



#endif // __LANDSCAPE_EDITOR_CONTROL_H__