#ifndef __OUTPUT_PANEL_CONTROL_H__
#define __OUTPUT_PANEL_CONTROL_H__

#include "DAVAEngine.h"

using namespace DAVA;

class OutputControl;
class CameraOutputControl;
class EditorScene;
class OutputPanelControl : public UIControl
{
    
public:
    OutputPanelControl(EditorScene *scene, const Rect & rect);
    virtual ~OutputPanelControl();
    
    virtual void WillAppear();
	virtual void Update(float32 timeElapsed);

    virtual void SetRect(const Rect &rect, bool rectInAbsoluteCoordinates = false);

    void UpdateCamera();
    
protected:

    //menu
    void CreateButtons();
    void ReleaseButtons();
    
    UIButton * btnOutput;
    UIButton * btnCamera;
    UIButton * btnClear;
    
    void OnOutputPressed(BaseObject * obj, void *, void *);
    void OnCameraPressed(BaseObject * obj, void *, void *);
    void OnClearPressed(BaseObject * obj, void *, void *);
    
    
    OutputControl *logControl;
    CameraOutputControl *cameraLog;
};



#endif // __OUTPUT_PANEL_CONTROL_H__