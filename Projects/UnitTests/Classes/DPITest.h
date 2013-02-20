//
//  InputTest.h
//  TemplateProjectMacOS
//
//  Created by adebt on 1/29/13.
//
//

#ifndef __TemplateProjectMacOS__DPITest__
#define __TemplateProjectMacOS__DPITest__

#include "DAVAEngine.h"

using namespace DAVA;

#include "TestTemplate.h"

class DPITest: public TestTemplate<DPITest>
{
public:
	DPITest();

	virtual void LoadResources();
	virtual void UnloadResources();
	virtual bool RunTest(int32 testNum);
	
	void TestFunction(PerfFuncData * data);
	
private:
	void ButtonPressed(BaseObject *obj, void *data, void *callerData);
	
private:
	UIStaticText* staticText;
	UIButton* testButton;
	
    bool testFinished;
};

#endif /* defined(__TemplateProjectMacOS__InputTest__) */
