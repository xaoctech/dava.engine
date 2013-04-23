#ifndef LevelPerformanceTest_ResultScreen_h
#define LevelPerformanceTest_ResultScreen_h

#include "DAVAEngine.h"
#include "LandscapeTestData.h"

enum eResultScreenState
{
	RESULT_STATE_NORMAL = 0,
	RESULT_STATE_MAKING_SCREEN_SHOT,
	RESULT_STATE_FINISHED
};

class ResultScreen: public DAVA::UIScreen
{
public:
	ResultScreen(const LandscapeTestData& testData, const DAVA::FilePath& filename, DAVA::Texture* landscapeTexture);
	~ResultScreen();
	
	virtual void LoadResources();
	virtual void UnloadResources();
	virtual void WillAppear();
	virtual void WillDisappear();
	
	virtual void Update(DAVA::float32 timeElapsed);
	virtual void Draw(const DAVA::UIGeometricData &geometricData);
	
	virtual void Input(DAVA::UIEvent * touch);

	eResultScreenState GetState() const {return state;};
	bool IsFinished() const {return isFinished;};
private:
	ResultScreen();
	ResultScreen& operator=(const ResultScreen&);
    
    void PrepareSprite();
	
	DAVA::Vector2 GetVecInRect(const DAVA::Rect & rect, DAVA::float32 angleInRad);

	void DrawStatImage(DAVA::Rect rect);
    
    void SaveResults();
    
	DAVA::Texture* texture;
	DAVA::Sprite* textureSprite;
    DAVA::Sprite* resultSprite;

	DAVA::FilePath filename;
	const LandscapeTestData& testData;
	eResultScreenState state;
	bool isFinished;
	
	DAVA::UIStaticText *fileNameText;
	DAVA::UIStaticText *statText[3];
	DAVA::UIStaticText *tapToContinue;
	DAVA::UIStaticText *screenshotText;
	
	DAVA::int32 testCount;
	DAVA::int32 testNumber;
};

#endif
