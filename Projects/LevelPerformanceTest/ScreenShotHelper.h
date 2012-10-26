#ifndef LevelPerformanceTest_ScreenShotHelper_h
#define LevelPerformanceTest_ScreenShotHelper_h

#include "DAVAEngine.h"

class ScreenShotHelper: public DAVA::Singleton<ScreenShotHelper> {
public:
	void MakeScreenShot();
	bool IsFinished();
	DAVA::String GetFileName();
	
private:
	bool isFinished;
	DAVA::String savedFileName;
};

#endif
