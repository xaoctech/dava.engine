//
//  InputTest.cpp
//  TemplateProjectMacOS
//
//  Created by adebt on 1/29/13.
//
//

#include "InputTest.h"

using namespace DAVA;

InputTest::InputTest() :
 TestTemplate<InputTest>("InputTest")
{
	textField = NULL;
	staticText = NULL;
	testButton = NULL;
	
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
	
	AddControl(testButton);
}

void InputTest::UnloadResources()
{
	SafeRelease(testButton);
	SafeRelease(textField);
	SafeRelease(staticText);
}

void InputTest::TestFunction(PerfFuncData * data)
{
	return;
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

