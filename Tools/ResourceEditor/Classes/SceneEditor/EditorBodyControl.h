#ifndef __EDITOR_BODY_CONTROL_H__
#define __EDITOR_BODY_CONTROL_H__

#include "DAVAEngine.h"
#include "CameraController.h"
#include "EditMatrixControl.h"
#include "../EditorScene.h"

#include "SceneNodeIDs.h"
#include "NodesPropertyControl.h"
#include "ModificationPopUp.h"

using namespace DAVA;

class SceneInfoControl;

class BeastManager;
class OutputPanelControl;
class EditorBodyControl : 
        public UIControl, 
        public UIHierarchyDelegate, 
        public NodesPropertyDelegate
{
    enum eConst
    {
        SCENE_OFFSET = 10, 
        CELL_HEIGHT = 20,
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
	
    enum eViewPortIDs
    {
        EVPID_IPHONE = 0,
        EVPID_RETINA,
        EVPID_IPAD,
        EVPID_DEFAULT,

        EVPID_COUNT
    };

    
    
public:
    EditorBodyControl(const Rect & rect);
    virtual ~EditorBodyControl();
    
    virtual void WillAppear();
	virtual void Update(float32 timeElapsed);
    virtual void Input(UIEvent * touch);

    void OpenScene(const String &pathToFile, bool editScene);
    void ReloadRootScene(const String &pathToFile);
    void ReloadNode(SceneNode *node, const String &pathToFile);
    
    void ShowProperties(bool show);
    bool PropertiesAreShown();

    void ShowSceneGraph(bool show);
    bool SceneGraphAreShown();

    void ShowDataGraph(bool show);
    bool DataGraphAreShown();

    
    void UpdateLibraryState(bool isShown, int32 width);

	void BeastProcessScene();
    virtual void DrawAfterChilds(const UIGeometricData &geometricData);
	    
    EditorScene * GetScene();
    void AddNode(SceneNode *node);
    
    SceneNode *GetSelectedSGNode(); //Scene Graph node
    
    virtual void NodesPropertyChanged();

    void CreateScene(bool withCameras);
    void ReleaseScene();
    void Refresh();
    void RefreshProperties();
    
    const String &GetFilePath();
    void SetFilePath(const String &newFilePath);
    
    void SetViewPortSize(int32 viewportID);
    bool ControlsAreLocked();

	void PushDebugCamera();
	void PopDebugCamera();

    void ToggleSceneInfo();
	
protected:

    void ResetSelection();
    void DebugInfo();
    
	void CreateModificationPanel(void);
    void ReleaseModificationPanel();
	void OnModificationPressed(BaseObject * object, void * userData, void * callerData);
	void OnModificationPopUpPressed(BaseObject * object, void * userData, void * callerData);
	void OnModePressed(BaseObject * object, void * userData, void * callerData);
	void UpdateModState(void);
	void PrepareModMatrix(const Vector2 & point);

	
    virtual bool IsNodeExpandable(UIHierarchy *forHierarchy, void *forNode);
    virtual int32 ChildrenCount(UIHierarchy *forHierarchy, void *forParent);
    virtual void *ChildAtIndex(UIHierarchy *forHierarchy, void *forParent, int32 index);
    virtual UIHierarchyCell *CellForNode(UIHierarchy *forHierarchy, void *node);
    virtual void OnCellSelected(UIHierarchy *forHierarchy, UIHierarchyCell *selectedCell);
    virtual void DragAndDrop(void *who, void *target);

    //left Panel
    void CreateLeftPanel();
    void ReleaseLeftPanel();
    
	void ReleaseHelpPanel();
	void CreateHelpPanel();

	void PlaceOnLandscape();
	
    UIControl *leftPanelSceneGraph;
    UIHierarchy * sceneGraphTree;
    void OnLookAtButtonPressed(BaseObject * obj, void *, void *);
    void OnRemoveNodeButtonPressed(BaseObject * obj, void *, void *);
    void OnEnableDebugFlagsPressed(BaseObject * obj, void *, void *);
    void OnBakeMatricesPressed(BaseObject * obj, void *, void *);
    void OnRefreshSceneGraph(BaseObject * obj, void *, void *);
	
	Vector3 GetIntersection(const Vector3 & start, const Vector3 & dir, const Vector3 & planeN, const Vector3 & planePos);
	void InitMoving(const Vector2 & point);
	void GetCursorVectors(Vector3 * from, Vector3 * dir, const Vector2 &point);
	
	
    UIControl *leftPanelDataGraph;
    UIHierarchy * dataGraphTree;
    enum eDataNodesIDs
    {
        EDNID_MATERIAL = 0,
        EDNID_MESH,
        EDNID_SCENE,
        
        EDNID_COUNT
    };
    
    DataNode *dataNodes[EDNID_COUNT];
    void RefreshDataGraph();
    void OnRefreshDataGraph(BaseObject * obj, void *, void *);

    //scene controls
    EditorScene * scene;
	Camera * activeCamera;
    UI3DView * scene3dView;
//    Max3dCameraController * cameraController;
    WASDCameraController * cameraController;
    // Node preview information
    void CreatePropertyPanel();
    void ReleasePropertyPanel();
    void UpdatePropertyPanel();
	void ToggleHelp(void);
	void AddHelpText(const wchar_t * text, float32 & y);
	
	

    UIControl *rightPanel;
    SceneNode * selectedSceneGraphNode;
    DataNode * selectedDataGraphNode;
    UIHierarchyCell *savedTreeCell;

    NodesPropertyControl *nodesPropertyPanel;
    //
    
    UIButton *refreshButton;
    void OnRefreshPressed(BaseObject * obj, void *, void *);
    
    // touch
    float32 currentTankAngle;
	bool inTouch;
	Vector2 touchStart;
	
	float32 startRotationInSec;

	//beast
	BeastManager * beastManager;

	UIButton *btnMod[3];
	UIButton *btnAxis[3];
	UIButton *btnPopUp;
	UIButton *btnModeSelection;
	UIButton *btnModeModification;
	UIButton *btnPlaceOn;
	
	UIControl *modificationPanel;
	eModState modState;
	eModAxis modAxis;
	Matrix4 startTransform;
	Matrix4 currTransform;
	Vector3 rotationCenter;
	bool isDrag;
	bool isModeModification;
	DraggableDialog *helpDialog;

	
	
	float32 axisSign[3];
	
    //OutputPanelControl
    OutputPanelControl *outputPanel;
	
	float32 moveKf;
    
    String mainFilePath;
    
    void ChangeControlWidthRight(UIControl *c, float32 width);
    void ChangeControlWidthLeft(UIControl *c, float32 width);
    
    void SelectNodeAtTree(SceneNode *node);

	Rect propertyPanelRect;
	void RecreatePropertiesPanelForNode(SceneNode *node);
	ModificationPopUp * modificationPopUp;
	

	//for moving object
	Vector3 startDragPoint;
	Vector3 planeNormal;
	
	Matrix4 translate1, translate2;

	SceneNode * mainCam;
	SceneNode * debugCam;
    
    struct AddedNode
    {
        SceneNode *nodeToAdd;
        SceneNode *nodeToRemove;
        SceneNode *parent;
    };
    Vector<AddedNode> nodesToAdd;
	
	//	Vector3 res = GetIntersection(Vector3(0,0,10), Vector3(0,0,-1), Vector3(0,0,1), Vector3(0,0,1));
	//
	//	Logger::Debug("intersection result %f %f %f", res.x, res.y, res.z);

	
    eViewPortIDs currentViewPortID;
    
    SceneInfoControl *sceneInfoControl;
};



#endif // __EDITOR_BODY_CONTROL_H__