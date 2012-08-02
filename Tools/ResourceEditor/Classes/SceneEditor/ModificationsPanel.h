#ifndef __MODIFICATIONS_PANEL_H__
#define __MODIFICATIONS_PANEL_H__

#include "DAVAEngine.h"

using namespace DAVA;

class ModificationsPanelDelegate
{
public: 
    
    virtual void OnPlaceOnLandscape() = 0;
    
};

class ModificationPopUp;
class EditorScene;
class ModificationsPanel: public UIControl
{
    enum eConst
    {
        BUTTON_W = 20,
        BUTTON_B = 5 
    };
    
    
public:

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
    ModificationsPanel(ModificationsPanelDelegate *newDelegate, const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false);
    virtual ~ModificationsPanel();
    
	virtual void Update(float32 timeElapsed);
    virtual void Input(UIEvent * touch);

    void SetScene(EditorScene *scene);
    void NodeSelected(SceneNode *newNode);
    
    Vector3 GetPlaneNormal();
    
    eModState GetModState();
    eModAxis GetModAxis();
    
    bool IsModificationMode();
    void IsModificationMode(bool value);

    bool IsLandscapeRelative();
    void IsLandscapeRelative(bool value);

	void OnReloadScene();

    
protected:
    
    void PlaceOnLandscape();

	void OnModificationPressed(BaseObject * object, void * userData, void * callerData);
    void OnModificationPopUpPressed(BaseObject * object, void * userData, void * callerData);
	void OnModePressed(BaseObject * object, void * userData, void * callerData);
	void OnCollisionPressed(BaseObject * object, void * userData, void * callerData);
	
	void OnLandscapeRelative(BaseObject * object, void * userData, void * callerData);

	void UpdateModState(void);
	void PrepareModMatrix(const Vector2 & point);
	void ChangeCollisionModeShow(SceneNode * node);

	UIButton *btnMod[3];
	UIButton *btnAxis[3];
	UIButton *btnPopUp;
	UIButton *btnModeSelection;
	UIButton *btnModeModification;
	UIButton *btnModeCollision;
	UIButton *btnPlaceOn;
	UIButton *btnLandscape;
	
	eModState modState;
	eModAxis modAxis;

	bool isModeModification;
	bool isLandscapeRelative;
	bool isModeCollision;

    UIControl * modificationPanel;
	ModificationPopUp * modificationPopUp;
	
    ModificationsPanelDelegate *delegate;
    EditorScene *workingScene;
};



#endif // __MODIFICATIONS_PANEL_H__
