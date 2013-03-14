#ifndef SurfaceTester_Test_h
#define SurfaceTester_Test_h

#include "DAVAEngine.h"
#include "LandscapeTestData.h"

using namespace DAVA;

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
	
	Texture* GetLandscapeTexture();

	const String GetFileName() const;
	const String GetFilePath() const;

	const LandscapeTestData& GetLandscapeTestData() const {return testData;};
private:
	Test();
	
	static int32 globalScreenId;
	int32 screenId;
	
	int32 skipFrames;

	String fullName;
	
	float32 time;
    
	LandscapeTestData testData;
    Vector3 curCameraPosition;
    uint32 nextRectNum;
    float32 curCameraAngle;
    LinearAnimation<Vector3>* camMoveAnimation;
    LinearAnimation<float32>* camRotateAnimation;

    Vector<DAVA::Rect> rectSequence;
	FpsStatItem fpsStatItem;

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
	inline Landscape* GetLandscape();
};

#endif
