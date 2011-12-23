#ifndef __EDITOR_BODY_CONTROL_H__
#define __EDITOR_BODY_CONTROL_H__

#include "DAVAEngine.h"
#include "CameraController.h"
#include "PropertyPanel.h"
#include "EditMatrixControl.h"
#include "../EditorScene.h"

using namespace DAVA;

class BeastManager;
class EditorBodyControl : public UIControl, public UIHierarchyDelegate
{
    enum eConst
    {
        SCENE_OFFSET = 10, 
        LEFT_SIDE_WIDTH = 200,
        RIGHT_SIDE_WIDTH = 200,
        CELL_HEIGHT = 20,
        MATRIX_HEIGHT = 100,
        BUTTON_HEIGHT = 20,
    };

	enum eModState
    {
        MOD_MOVE = 0, 
        MOD_ROTATE,
        MOD_SCALE
	};

	enum eModAxis
    {
        AXIS_X = 0, 
        AXIS_Y,
        AXIS_Z
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

    void ShowSceneGraph(bool show);
    bool SceneGraphAreShown();

    void UpdateLibraryState(bool isShown, int32 width);

	void BeastProcessScene();
    virtual void DrawAfterChilds(const UIGeometricData &geometricData);
	
protected:

    void CreateScene();
    void ReleaseScene();
    
    void CreatePropertyPanel();
    void ReleasePropertyPanel();
    
	void CreateModificationPanel(void);
	void OnModificationPressed(BaseObject * object, void * userData, void * callerData);
	void UpdateModState(void);
	void PrepareModMatrix(float32 value);

	
    virtual bool IsNodeExpandable(UIHierarchy *forHierarchy, void *forNode);
    virtual int32 ChildrenCount(UIHierarchy *forHierarchy, void *forParent);
    virtual void *ChildAtIndex(UIHierarchy *forHierarchy, void *forParent, int32 index);
    virtual UIHierarchyCell *CellForNode(UIHierarchy *forHierarchy, void *node);
    virtual void OnCellSelected(UIHierarchy *forHierarchy, UIHierarchyCell *selectedCell);
    
    
    UIHierarchy * sceneTree;
    
    //scene controls
    EditorScene * scene;
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
	
	float32 startRotationInSec;

	//beast
	BeastManager * beastManager;

	UIButton *btnMod[3];
	UIButton *btnAxis[3];

	UIControl *modificationPanel;
	eModState modState;
	eModAxis modAxis;
	Matrix4 startTransform;
	Matrix4 currTransform;
};



#endif // __EDITOR_BODY_CONTROL_H__