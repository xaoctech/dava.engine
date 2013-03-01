//
//  InputTest.cpp
//  TemplateProjectMacOS
//
//  Created by adebt on 1/29/13.
//
//

#include "InputTest.h"

using namespace DAVA;

static const float INPUT_TEST_AUTO_CLOSE_TIME = 30.0f;

InputTest::InputTest() :
 TestTemplate<InputTest>("InputTest")
{
	textField = NULL;
	staticText = NULL;
	testButton = NULL;
	
	onScreenTime = 0.0f;
	testFinished = false;
	
	RegisterFunction(this, &InputTest::TestFunction, Format("InputTest"), NULL);
}

void InputTest::LoadResources()
{
	GetBackground()->SetColor(Color(1.f, 0, 0, 1));
	
	Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
	font->SetSize(20);
    font->SetColor(Color::White());
	
	textField = new UITextField(Rect(0, 0, 512, 100));
#ifdef __DAVAENGINE_IPHONE__
	textField->SetFontColor(1.f, 1.f, 1.f, 1.f);
#else
	textField->SetFont(font);
#endif
	textField->SetText(L"textField");
	textField->SetDebugDraw(true);
	textField->SetDelegate(new UITextFieldDelegate());
	AddControl(textField);
	
	textField = new UITextField(Rect(600, 10, 100, 100));
#ifdef __DAVAENGINE_IPHONE__
	textField->SetFontColor(1.f, 1.f, 1.f, 1.f);
#else
	textField->SetFont(font);
#endif
	textField->SetText(L"textField");
	textField->SetDebugDraw(true);
	AddControl(textField);

	textField = new UITextField(Rect(750, 10, 100, 500));
#ifdef __DAVAENGINE_IPHONE__
	textField->SetFontColor(1.f, 1.f, 1.f, 1.f);
#else
	textField->SetFont(font);
#endif
	textField->SetText(L"textField");
	textField->SetDebugDraw(true);
	AddControl(textField);

	testButton = new UIButton(Rect(0, 300, 300, 30));
	testButton->SetStateFont(0xFF, font);
	testButton->SetStateText(0xFF, L"Finish Test");
	testButton->SetDebugDraw(true);
	testButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &InputTest::ButtonPressed));

	webView1 = new UIWebView(Rect(5, 105, 500, 190));
	webView1->OpenURL("http://www.google.com");
	AddControl(webView1);

	webView2 = new UIWebView(Rect(305, 300, 440, 190));
	webView2->OpenURL("http://www.apple.com");
	AddControl(webView2);

	AddControl(testButton);
}

void InputTest::UnloadResources()
{
	RemoveAllControls();

	SafeRelease(testButton);
	SafeRelease(textField);
	SafeRelease(staticText);
	
	SafeRelease(webView1);
	SafeRelease(webView2);
}

void InputTest::TestFunction(PerfFuncData * data)
{
	return;
}

void InputTest::DidAppear()
{
    onScreenTime = 0.f;
}

void InputTest::Update(float32 timeElapsed)
{
    onScreenTime += timeElapsed;
    if(onScreenTime > INPUT_TEST_AUTO_CLOSE_TIME)
    {
        testFinished = true;
    }

    TestTemplate<InputTest>::Update(timeElapsed);
}

bool InputTest::RunTest(int32 testNum)
{
	TestTemplate<InputTest>::RunTest(testNum);
	return testFinished;
}


void InputTest::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
	testFinished = true;
}

