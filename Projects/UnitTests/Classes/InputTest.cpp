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
	
	RegisterFunction(this, &InputTest::TestFunction, Format("InputTest of %s", "asdasd"), NULL);
}

void InputTest::LoadResources()
{
	GetBackground()->SetColor(Color(1.f, 0, 0, 1));
	
	Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
	font->SetSize(20);
    font->SetColor(Color::White());
	
	staticText = new UIStaticText(Rect(0, 0, 512, 200));
    staticText->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
    staticText->SetMultiline(true);
    staticText->SetFont(font);
	staticText->SetText(L"staticText");
	staticText->SetDebugDraw(true);
    AddControl(staticText);
	
	textField = new UITextField(Rect(0, 200, 512, 200));
#ifndef __DAVAENGINE_IPHONE__
	textField->SetFont(font);
#endif
	textField->SetText(L"textField");
	textField->SetDebugDraw(true);
	AddControl(textField);
	
	testButton = new UIButton(Rect(0, 400, 512, 100));
	testButton->SetStateFont(0xFF, font);
	testButton->SetStateText(0xFF, L"UIButton");
	testButton->SetDebugDraw(true);
	
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
