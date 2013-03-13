//
//  InputTest.h
//  TemplateProjectMacOS
//
//  Created by adebt on 1/29/13.
//
//

#ifndef __EMAIL_TEST_H__
#define __EMAIL_TEST_H__

#include "DAVAEngine.h"

using namespace DAVA;

#include "TestTemplate.h"

class EMailTest: public TestTemplate<EMailTest>
{
	static float32 AUTO_CLOSE_TIME;

public:
	EMailTest();

	virtual void LoadResources();
	virtual void UnloadResources();
	virtual bool RunTest(int32 testNum);
	
	virtual void DidAppear();	
	virtual void Update(float32 timeElapsed);
	
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
		
	float32 onScreenTime;
};

#endif /* defined(__EMAIL_TEST_H__) */
