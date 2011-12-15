#ifndef __EDITOR_BODY_CONTROL_H__
#define __EDITOR_BODY_CONTROL_H__

#include "DAVAEngine.h"
#include "CameraController.h"
#include "PropertyPanel.h"
#include "EditMatrixControl.h"

using namespace DAVA;

class BeastManager;

class EditorBodyControl : public UIControl, public UIHierarchyDelegate
{
    enum eConst
    {
        LEFT_SIDE_WIDTH = 200,
        RIGHT_SIDE_WIDTH = 300,
        SCENE_HEIGHT = 400,
        CELL_HEIGHT = 20,
        MATRIX_HEIGHT = 100,
        BUTTON_HEIGHT = 20,
    };
    
public:
    EditorBodyControl(const Rect & rect);
    virtual ~EditorBodyControl();
    
    virtual void WillAppear();
	virtual void Update(float32 timeElapsed);
    virtual void Input(UIEvent * touch);

    void OpenScene(const String &pathToFile);
    
    void ShowProperties(bool show);
    bool PropertiesAreShown();
    
protected:

    void CreateScene();
    void ReleaseScene();
    
    void CreatePropertyPanel();
    void ReleasePropertyPanel();
    
    UIButton *CreateButton(Rect r, const WideString &text);
    
    
    virtual bool IsNodeExpandable(UIHierarchy *forHierarchy, void *forNode);
    virtual int32 ChildrenCount(UIHierarchy *forHierarchy, void *forParent);
    virtual void *ChildAtIndex(UIHierarchy *forHierarchy, void *forParent, int32 index);
    virtual UIHierarchyCell *CellForNode(UIHierarchy *forHierarchy, void *node);
    virtual void OnCellSelected(UIHierarchy *forHierarchy, UIHierarchyCell *selectedCell);
    
    
    UIHierarchy * sceneTree;
    
    //scene controls
    Scene * scene;
	Camera * activeCamera;
    UI3DView * scene3dView;
    WASDCameraController * cameraController;
    
    // Node preview information
    SceneNode * selectedNode;
    PropertyPanel * activePropertyPanel;
    EditMatrixControl * localMatrixControl;
    EditMatrixControl * worldMatrixControl;
    void OnLocalTransformChanged(BaseObject * object, void * userData, void * callerData);
    
    
    UIStaticText * nodeName;
    UIStaticText * nodeCenter;
    UIStaticText * nodeBoundingBoxMin;
    UIStaticText * nodeBoundingBoxMax;
    UIButton * lookAtButton;
    UIButton * removeNodeButton;
    UIButton * enableDebugFlagsButton;
    
    void OnLookAtButtonPressed(BaseObject * obj, void *, void *);
    void OnRemoveNodeButtonPressed(BaseObject * obj, void *, void *);
    void OnEnableDebugFlagsPressed(BaseObject * obj, void *, void *);
    
    // touch
    float32 currentTankAngle;
	bool inTouch;
	Vector2 touchStart;
	Vector2 touchCurrent;
	float32 touchTankAngle;
	float32 rotationSpeed;
	
	float32 startRotationInSec;

    // general
    Font *fontLight;
    Font *fontDark;

	//beast
	BeastManager * beastManager;
};



#endif // __EDITOR_BODY_CONTROL_H__