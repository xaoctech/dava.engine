#ifndef LevelPerformanceTest_ResultScreen_h
#define LevelPerformanceTest_ResultScreen_h

#include "DAVAEngine.h"

using namespace DAVA;

enum eResultScreenState
{
	RESULT_STATE_NORMAL = 0,
	RESULT_STATE_MAKING_SCREEN_SHOT,
	RESULT_STATE_FINISHED
};

class ResultScreen: public DAVA::UIScreen
#ifdef __DAVAENGINE_IPHONE__
  , public SaveToSystemPhotoCallbackReceiver
#endif
{
public:
	ResultScreen(const Vector<float32>* const fpsStatistics, const String& filename, int32 testCount, int32 testNumber);
	~ResultScreen();
	
	virtual void LoadResources();
	virtual void UnloadResources();
	virtual void WillAppear();
	virtual void WillDisappear();
	
	virtual void Update(float32 timeElapsed);
	virtual void Draw(const UIGeometricData &geometricData);
    virtual void SystemDraw(const UIGeometricData &geometricData);
	
	virtual void Input(DAVA::UIEvent * touch);
    
#ifdef __DAVAENGINE_IPHONE__
    virtual void SaveToSystemPhotosFinished();
#endif

	eResultScreenState GetState() const {return state;};
	bool IsFinished() const {return isFinished;};
private:
	ResultScreen();
	ResultScreen& operator=(const ResultScreen&);
    
    void PrepareSprite();
	
	void DrawStatImage(Vector<float32> *v, DAVA::Rect rect);
	Color PickColor(float32 fps) const;
    
    Sprite* resultSprite;

	String filename;
	Vector<float32> data[3];
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
