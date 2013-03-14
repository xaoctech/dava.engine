//
//  InputTest.h
//  TemplateProjectMacOS
//
//  Created by adebt on 1/29/13.
//
//

#ifndef __DPI_TEST_H__
#define __DPI_TEST_H__

#include "DAVAEngine.h"

using namespace DAVA;

#include "TestTemplate.h"

class DPITest: public TestTemplate<DPITest>
{
    static float32 AUTO_CLOSE_TIME;
    
public:
	DPITest();

	virtual void LoadResources();
	virtual void UnloadResources();
	virtual bool RunTest(int32 testNum);
	
	virtual void DidAppear();
	virtual void Update(float32 timeElapsed);
    
    void TestFunction(PerfFuncData * data);
	
private:
	void ButtonPressed(BaseObject *obj, void *data, void *callerData);
	
private:
	UIStaticText* staticText;
	UIButton* testButton;
	
    bool testFinished;

    float32 onScreenTime;
};

#endif /* defined(__DPI_TEST_H__) */
