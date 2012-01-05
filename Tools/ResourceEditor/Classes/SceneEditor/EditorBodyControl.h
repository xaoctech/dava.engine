#ifndef __EDITOR_BODY_CONTROL_H__
#define __EDITOR_BODY_CONTROL_H__

#include "DAVAEngine.h"
#include "CameraController.h"
#include "PropertyPanel.h"
#include "EditMatrixControl.h"
#include "../EditorScene.h"

#include "SceneNodeIDs.h"
#include "NodePropertyControl.h"

using namespace DAVA;

class BeastManager;
class OutputPanelControl;
class NodePropertyControl;
class EditorBodyControl : public UIControl, public UIHierarchyDelegate, public NodePropertyDelegate
{
    enum eConst
    {
        SCENE_OFFSET = 10, 
        CELL_HEIGHT = 20,
        MATRIX_HEIGHT = 100,
        OUTPUT_PANEL_HEIGHT = 200,
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
        AXIS_Z,
        AXIS_XY,
        AXIS_YZ,
        AXIS_XZ,
		AXIS_COUNT
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
	    
    EditorScene * GetScene();
    void AddNode(SceneNode *node);
    
    virtual void NodePropertyChanged();

    void CreateScene(bool withCameras);
    void ReleaseScene();
    void Refresh();

protected:

    void DebugInfo();
    
	void CreateModificationPanel(void);
    void ReleaseModificationPanel();
	void OnModificationPressed(BaseObject * object, void * userData, void * callerData);
	void UpdateModState(void);
	void PrepareModMatrix(float32 a, float32 b);

	
    virtual bool IsNodeExpandable(UIHierarchy *forHierarchy, void *forNode);
    virtual int32 ChildrenCount(UIHierarchy *forHierarchy, void *forParent);
    virtual void *ChildAtIndex(UIHierarchy *forHierarchy, void *forParent, int32 index);
    virtual UIHierarchyCell *CellForNode(UIHierarchy *forHierarchy, void *node);
    virtual void OnCellSelected(UIHierarchy *forHierarchy, UIHierarchyCell *selectedCell);
    
    //left Panel
    void CreateLeftPanel();
    void ReleaseLeftPanel();
    
    UIControl *leftPanel;
    UIHierarchy * sceneTree;
    UIButton * lookAtButton;
    UIButton * removeNodeButton;
    UIButton * enableDebugFlagsButton;
    
    void OnLookAtButtonPressed(BaseObject * obj, void *, void *);
    void OnRemoveNodeButtonPressed(BaseObject * obj, void *, void *);
    void OnEnableDebugFlagsPressed(BaseObject * obj, void *, void *);
    
    
    //scene controls
    EditorScene * scene;
	Camera * activeCamera;
    UI3DView * scene3dView;
    WASDCameraController * cameraController;
    
    // Node preview information
    void CreatePropertyPanel();
    void ReleasePropertyPanel();
    void UpdatePropertyPanel();
    UIControl *rightPanel;
    SceneNode * selectedNode;
    UIHierarchyCell *savedTreeCell;
//    PropertyPanel * activePropertyPanel;
//    EditMatrixControl * localMatrixControl;
//    EditMatrixControl * worldMatrixControl;
//    void OnLocalTransformChanged(BaseObject * object, void * userData, void * callerData);
    
    NodePropertyControl *nodePropertyPanel[ECNID_COUNT + 1];
    NodePropertyControl *currentPropertyPanel;
    //
    
    UIButton *refreshButton;
    void OnRefreshPressed(BaseObject * obj, void *, void *);

    
    
//    UIStaticText * nodeName;
//    UIStaticText * nodeCenter;
//    UIStaticText * nodeBoundingBoxMin;
//    UIStaticText * nodeBoundingBoxMax;
    
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

	float32 axisSign[3];
	
    //OutputPanelControl
    OutputPanelControl *outputPanel;
    
    void ChangeControlWidthRight(UIControl *c, float32 width);
    void ChangeControlWidthLeft(UIControl *c, float32 width);
};



#endif // __EDITOR_BODY_CONTROL_H__