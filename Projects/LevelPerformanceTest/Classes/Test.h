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
	
	int GetScreenId() const {return m_nScreenId;};
	
	bool IsFinished() const {return m_bIsFinished;};
	
	const Vector<float>* const GetStat() const {return m_FpsStat;};
	const String GetFileName() const;
private:
	Test();
	
	static int globalScreenId;
	int m_nScreenId;
	
	int m_nSkipFrames;
	
	String m_FullName;
	
	float32 m_fTime;
	
	float m_fVelocity;
	Vector3 m_VelocityDir;
	
	int m_nCurPathPointNum;
	Vector3 m_CurPathPoint;
	Vector3 m_NextPathPoint;
	
	int m_nNextFpsCheckPointNum;
	Vector3 m_NextFpsCheckPoint;
	
	float m_fFpsMin;
	float m_fFpsMid;
	float m_fFpsMax;
	int m_nFrameCount;
	
	Vector<Vector3> m_CenterPoints;
	Vector<Vector3> m_FpsCheckPoints;
	Vector<float> m_FpsStat[3];

	bool m_bIsFinished;
	
	void PreparePath();
	void PrepareFpsCheckPoints();
	void UpdatePathSegment();
	void UpdateFpsSegment();
	
	void ZeroCurFpsStat();

	inline UI3DView* GetSceneView();
	inline Scene* GetScene();
	inline Camera* GetCamera();
	inline LandscapeNode* GetLandscape();
};

#endif
