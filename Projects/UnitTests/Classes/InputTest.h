//
//  InputTest.h
//  TemplateProjectMacOS
//
//  Created by adebt on 1/29/13.
//
//

#ifndef __TemplateProjectMacOS__InputTest__
#define __TemplateProjectMacOS__InputTest__

#include "DAVAEngine.h"
using namespace DAVA;

#include "TestTemplate.h"

class InputTest: public TestTemplate<InputTest>
{
public:
	InputTest();

	virtual void LoadResources();
	virtual void UnloadResources();
	virtual bool RunTest(int32 testNum);
	
	void TestFunction(PerfFuncData * data);
	
private:
	UITextField* textField;
	UIStaticText* staticText;
	UIButton* testButton;
};

#endif /* defined(__TemplateProjectMacOS__InputTest__) */
