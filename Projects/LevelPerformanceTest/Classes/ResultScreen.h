#ifndef LevelPerformanceTest_ResultScreen_h
#define LevelPerformanceTest_ResultScreen_h

#include "DAVAEngine.h"

using namespace DAVA;

enum ResultScreenState {
	RESULT_STATE_NORMAL=0,
	RESULT_STATE_MAKING_SCREEN_SHOT,
	RESULT_STATE_FINISHED
};

class ResultScreen: public DAVA::UIScreen {
public:
	virtual void LoadResources();
	virtual void UnloadResources();
	virtual void WillAppear();
	virtual void WillDisappear();
	
	virtual void Update(float32 timeElapsed);
	virtual void Draw(const UIGeometricData &geometricData);
	
	virtual void Input(DAVA::UIEvent * touch);
	
	ResultScreen(const Vector<float>* const fpsStatistics, const String& filename, int testCount, int testNumber);

	ResultScreenState GetState() const {return state;};
	bool IsFinished() const {return isFinished;};
private:
	ResultScreen();
	ResultScreen& operator=(const ResultScreen&);
	
	void DrawStatImage(Vector<float> *v, DAVA::Rect rect);
	Color PickColor(float fps) const;

	String filename;
	Vector<float> data[3];
	ResultScreenState state;
	bool isFinished;
	
	UIStaticText *fileNameText;
	UIStaticText *statText[3];
	UIStaticText *tapToContinue;
	UIStaticText *screenshotText;
	
	int testCount;
	int testNumber;
};

#endif
