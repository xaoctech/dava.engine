//
//  InputTest.cpp
//  TemplateProjectMacOS
//
//  Created by adebt on 1/29/13.
//
//

#include "EMailTest.h"
#include "Network/MailSender.h"

using namespace DAVA;

float32 EMailTest::AUTO_CLOSE_TIME = 30.f;

EMailTest::EMailTest() :
 TestTemplate<EMailTest>("EMailTest")
{
	address = NULL;
	subject = NULL;
	text = NULL;
	sendMailBtn = NULL;
	finishTestBtn = NULL;
	
	testFinished = false;
	
	RegisterFunction(this, &EMailTest::TestFunction, Format("EMailTest"), NULL);
	
	 onScreenTime = 0.f;
}

void EMailTest::LoadResources()
{
	Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
	font->SetSize(20);
    font->SetColor(Color::White());
	
	UIStaticText* staticText = NULL;
	staticText = new UIStaticText(Rect(0, 10, 100, 30));
    staticText->SetAlign(ALIGN_RIGHT | ALIGN_VCENTER);
    staticText->SetMultiline(true);
    staticText->SetFont(font);
	staticText->SetText(L"address");
    AddControl(staticText);
	SafeRelease(staticText);

	staticText = new UIStaticText(Rect(0, 50, 100, 30));
    staticText->SetAlign(ALIGN_RIGHT | ALIGN_VCENTER);
    staticText->SetMultiline(true);
    staticText->SetFont(font);
	staticText->SetText(L"subject");
    AddControl(staticText);
	SafeRelease(staticText);

	staticText = new UIStaticText(Rect(0, 90, 100, 200));
    staticText->SetAlign(ALIGN_RIGHT | ALIGN_VCENTER);
    staticText->SetMultiline(true);
    staticText->SetFont(font);
	staticText->SetText(L"text");
    AddControl(staticText);
	SafeRelease(staticText);
	
	address = new UITextField(Rect(100, 10, 600, 30));
#ifdef __DAVAENGINE_IPHONE__
	//address->SetFontColor(1.f, 1.f, 1.f, 1.f);
#else
	address->SetFont(font);
#endif

	address->SetText(L"test@test.com");
	address->SetDebugDraw(true);
	address->SetDelegate(new UITextFieldDelegate());
	AddControl(address);

	subject = new UITextField(Rect(100, 50, 600, 30));
#ifdef __DAVAENGINE_IPHONE__
	//subject->SetFontColor(1.f, 1.f, 1.f, 1.f);
#else
	subject->SetFont(font);
#endif

	subject->SetText(L"test subject");
	subject->SetDebugDraw(true);
	subject->SetDelegate(new UITextFieldDelegate());
	AddControl(subject);

	text = new UITextField(Rect(100, 90, 600, 200));
#ifdef __DAVAENGINE_IPHONE__
	//text->SetFontColor(1.f, 1.f, 1.f, 1.f);
#else
	text->SetFont(font);
#endif

	text->SetText(L"test text");
	text->SetDebugDraw(true);
	text->SetDelegate(new UITextFieldDelegate());
	AddControl(text);

	sendMailBtn = new UIButton(Rect(10, 310, 300, 30));
	sendMailBtn->SetStateFont(0xFF, font);
	sendMailBtn->SetStateText(0xFF, L"Send Mail");
	sendMailBtn->SetDebugDraw(true);
	sendMailBtn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EMailTest::ButtonPressed));
	AddControl(sendMailBtn);
	
	finishTestBtn = new UIButton(Rect(320, 310, 300, 30));
	finishTestBtn->SetStateFont(0xFF, font);
	finishTestBtn->SetStateText(0xFF, L"Finish test");
	finishTestBtn->SetDebugDraw(true);
	finishTestBtn->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &EMailTest::ButtonPressed));
	AddControl(finishTestBtn);
}

void EMailTest::UnloadResources()
{
	RemoveAllControls();
	
	SafeRelease(address);
	SafeRelease(subject);
	SafeRelease(text);
	SafeRelease(sendMailBtn);
	SafeRelease(finishTestBtn);
}

void EMailTest::DidAppear()
{
    onScreenTime = 0.f;
}

void EMailTest::Update(float32 timeElapsed)
{
    onScreenTime += timeElapsed;
    if(onScreenTime > AUTO_CLOSE_TIME)
    {
        testFinished = true;
    }
    
    TestTemplate<EMailTest>::Update(timeElapsed);
}

void EMailTest::TestFunction(PerfFuncData * data)
{
	return;
}

bool EMailTest::RunTest(int32 testNum)
{
	TestTemplate<EMailTest>::RunTest(testNum);
	return testFinished;
}


void EMailTest::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
	if (obj == sendMailBtn)
	{
		MailSender::SendEmail(address->GetText(), subject->GetText(), text->GetText());
	}
	else if (obj == finishTestBtn)
	{
		testFinished = true;
	}
}

