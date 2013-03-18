#ifndef LevelPerformanceTest_ResultScreen_h
#define LevelPerformanceTest_ResultScreen_h

#include "DAVAEngine.h"
#include "LandscapeTestData.h"

using namespace DAVA;

enum eResultScreenState
{
	RESULT_STATE_NORMAL = 0,
	RESULT_STATE_MAKING_SCREEN_SHOT,
	RESULT_STATE_FINISHED
};

class ResultScreen: public DAVA::UIScreen
{
public:
	ResultScreen(const LandscapeTestData& testData, const String& filename, Texture* landscapeTexture);
	~ResultScreen();
	
	virtual void LoadResources();
	virtual void UnloadResources();
	virtual void WillAppear();
	virtual void WillDisappear();
	
	virtual void Update(float32 timeElapsed);
	virtual void Draw(const UIGeometricData &geometricData);
	
	virtual void Input(DAVA::UIEvent * touch);

	eResultScreenState GetState() const {return state;};
	bool IsFinished() const {return isFinished;};
private:
	ResultScreen();
	ResultScreen& operator=(const ResultScreen&);
    
    void PrepareSprite();
	
	void DrawStatImage(DAVA::Rect rect);
	void DrawMinFpsTargets(DAVA::Rect rect);
    
    void SaveResults();
    
	Texture* texture;
	Sprite* textureSprite;
    Sprite* resultSprite;

	String filename;
	const LandscapeTestData& testData;
	eResultScreenState state;
	bool isFinished;
	
	UIStaticText *fileNameText;
	UIStaticText *statText[3];
	UIStaticText *tapToContinue;
	UIStaticText *screenshotText;
	
	int32 testCount;
	int32 testNumber;
};

#endif
