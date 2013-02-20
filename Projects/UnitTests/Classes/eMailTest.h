//
//  InputTest.h
//  TemplateProjectMacOS
//
//  Created by adebt on 1/29/13.
//
//

#ifndef __TemplateProjectMacOS__eMailTest__
#define __TemplateProjectMacOS__eMailTest__

#include "DAVAEngine.h"

using namespace DAVA;

#include "TestTemplate.h"

class EMailTest: public TestTemplate<EMailTest>
{
public:
	EMailTest();

	virtual void LoadResources();
	virtual void UnloadResources();
	virtual bool RunTest(int32 testNum);
	
	void TestFunction(PerfFuncData * data);
	
private:
	void ButtonPressed(BaseObject *obj, void *data, void *callerData);
	
private:
	UITextField* address;
	UITextField* subject;
	UITextField* text;
	UIButton* sendMailBtn;
	UIButton* finishTestBtn;

	bool testFinished;
};

#endif /* defined(__TemplateProjectMacOS__InputTest__) */
