//
//  InputTest.cpp
//  TemplateProjectMacOS
//
//  Created by adebt on 1/29/13.
//
//

#include "DPITest.h"

using namespace DAVA;

float32 DPITest::AUTO_CLOSE_TIME = 30.f;

DPITest::DPITest() :
 TestTemplate<DPITest>("DPITest")
{
	staticText = NULL;
	testButton = NULL;
	
	testFinished = false;
	
	RegisterFunction(this, &DPITest::TestFunction, Format("DPITest"), NULL);
    
    onScreenTime = 0.f;
}

void DPITest::LoadResources()
{
	GetBackground()->SetColor(Color(1.f, 0, 0, 1));
	
	Font *font = FTFont::Create(FilePath("~res:/Fonts/korinna.ttf"));
    DVASSERT(font);
	font->SetSize(20);
    font->SetColor(Color::White());
	
	staticText = new UIStaticText(Rect(0, 0, 512, 100));
	staticText->SetFontColor(Color::White());
    staticText->SetFont(font);
	staticText->SetText(L"textField");
	staticText->SetDebugDraw(true);
	//staticText->SetDelegate(new UITextFieldDelegate());
    String str(Format("Detected DPI: %d", DAVA::Core::Instance()->GetScreenDPI()));
    staticText->SetText(StringToWString(str));
	AddControl(staticText);
	

	testButton = new UIButton(Rect(0, 300, 300, 30));
	testButton->SetStateFont(0xFF, font);
	testButton->SetStateText(0xFF, L"Finish Test");
	testButton->SetDebugDraw(true);
	testButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &DPITest::ButtonPressed));

	AddControl(testButton);
}

void DPITest::UnloadResources()
{
	RemoveAllControls();

	SafeRelease(testButton);
	SafeRelease(staticText);
}


void DPITest::DidAppear()
{
    onScreenTime = 0.f;
}

void DPITest::Update(float32 timeElapsed)
{
    onScreenTime += timeElapsed;
    if(onScreenTime > AUTO_CLOSE_TIME)
    {
        testFinished = true;
    }
    
    TestTemplate<DPITest>::Update(timeElapsed);
}

void DPITest::TestFunction(PerfFuncData * data)
{
	return;
}

bool DPITest::RunTest(int32 testNum)
{
	TestTemplate<DPITest>::RunTest(testNum);
	return testFinished;
}


void DPITest::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
	testFinished = true;
}

