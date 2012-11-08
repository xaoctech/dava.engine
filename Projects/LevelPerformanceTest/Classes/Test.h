#ifndef SurfaceTester_Test_h
#define SurfaceTester_Test_h

#include "DAVAEngine.h"

using namespace DAVA;

enum eStatFps
{
    STAT_MIN = 0,
    STAT_MID,
    STAT_MAX,
    
    STAT_COUNT
};

class Test: public DAVA::UIScreen
{
public:
	Test(const String& fullName);
	
	virtual void LoadResources();
	virtual void UnloadResources();
	virtual void WillAppear();
	virtual void WillDisappear();
	
	virtual void Update(float32 timeElapsed);
	virtual void Draw(const UIGeometricData &geometricData);
	
	virtual void Input(DAVA::UIEvent * touch);
	
	int32 GetScreenId() const {return screenId;};
	
	bool IsFinished() const {return isFinished;};
	
	const Vector<float32>* const GetStat() const {return fpsStat;};
	const String GetFileName() const;
private:
	Test();
	
	static int32 globalScreenId;
	int32 screenId;

	String fullName;
	
	float32 time;

	float32 fpsMin;
	float32 fpsMid;
	float32 fpsMax;
	int32 frameCount;
    
    float32 curFpsRectNum;
    Vector3 curCameraPosition;
    int32 nextRectNum;
    float32 curCameraAngle;
    LinearAnimation<Vector3>* camMoveAnimation;
    LinearAnimation<float32>* camRotateAnimation;

    Vector<DAVA::Rect> rectSequence;
	Vector<float32> fpsStat[STAT_COUNT];

	bool isFinished;
	
    Vector3 GetRealPoint(const Vector2& point);
    
	void PreparePath();
    void PrepareCameraAnimation();
    void PrepareFpsStat();
    void MoveToNextPoint();
    void SaveFpsStat();
	void ZeroCurFpsStat();
    void AnimationFinished(BaseObject*, void*, void*);

	inline UI3DView* GetSceneView();
	inline Scene* GetScene();
	inline Camera* GetCamera();
	inline LandscapeNode* GetLandscape();
};

#endif
