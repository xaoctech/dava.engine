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
#include "UI/UIWebView.h"

using namespace DAVA;

#include "TestTemplate.h"

class InputTest: public TestTemplate<InputTest>
{
public:
	InputTest();

	virtual void LoadResources();
	virtual void UnloadResources();
	virtual bool RunTest(int32 testNum);

	virtual void DidAppear();
	virtual void Update(float32 timeElapsed);

	void TestFunction(PerfFuncData * data);
	
private:
	void ButtonPressed(BaseObject *obj, void *data, void *callerData);
	
private:
	UITextField* textField;
	UIStaticText* staticText;
	UIButton* testButton;
	
	UIWebView* webView1;
	UIWebView* webView2;

	bool testFinished;
	float onScreenTime;
};

#endif /* defined(__TemplateProjectMacOS__InputTest__) */
