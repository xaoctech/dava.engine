#ifndef SurfaceTester_Test_h
#define SurfaceTester_Test_h

#include "DAVAEngine.h"

using namespace DAVA;

class Test: public DAVA::UIScreen {
public:
	Test(const String& fullName, int skipFrames=10);
	
	virtual void LoadResources();
	virtual void UnloadResources();
	virtual void WillAppear();
	virtual void WillDisappear();
	
	virtual void Update(float32 timeElapsed);
	virtual void Draw(const UIGeometricData &geometricData);
	
	virtual void Input(DAVA::UIEvent * touch);
	
	int GetScreenId() const {return screenId;};
	
	bool IsFinished() const {return isFinished;};
	
	const Vector<float>* const GetStat() const {return fpsStat;};
	const String GetFileName() const;
private:
	Test();
	
	static int globalScreenId;
	int screenId;
	
	int skipFrames;
	
	String fullName;
	
	float32 time;
	
	float velocity;
	Vector3 velocityDir;
	
	int curPathPointNum;
	Vector3 curPathPoint;
	Vector3 nextPathPoint;
	
	int nextFpsCheckPointNum;
	Vector3 nextFpsCheckPoint;
	
	float fpsMin;
	float fpsMid;
	float fpsMax;
	int frameCount;
	
	Vector<Vector3> centerPoints;
	Vector<Vector3> fpsCheckPoints;
	Vector<float> fpsStat[3];
	
	LandscapeNode *land;
	Camera *cam;
	
	bool isFinished;
	
	void PreparePath();
	void PrepareFpsCheckPoints();
	void UpdatePathSegment();
	void UpdateFpsSegment();
};

#endif
