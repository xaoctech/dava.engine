#include "TransparentWebViewTest.h"

static const float TRANSPARENT_WEB_VIEW_TEST_AUTO_CLOSE_TIME = 30.0f;

TransparentWebViewTest::TransparentWebViewTest()
:	TestTemplate<TransparentWebViewTest>("TransparentWebViewTest")
,	webView1(NULL)
,	webView2(NULL)
,	testFinished(false)
{
	RegisterFunction(this, &TransparentWebViewTest::TestFunction, Format("TransparentWebViewTest"), NULL);
}

void TransparentWebViewTest::LoadResources()
{
	webView1 = new UIWebView(Rect(5, 5, 500, 500));
	webView1->SetVisible(true);
	webView1->OpenURL("http://www.davaconsulting.com/");
	AddControl(webView1);

	FilePath srcDir("~res:/TestData/TransparentWebViewTest/");
	FilePath cpyDir = FileSystem::Instance()->GetCurrentDocumentsDirectory() + "TransparentWebViewTest/";
	FileSystem::Instance()->DeleteDirectory(cpyDir);
	FileSystem::Instance()->CreateDirectory(cpyDir);
	FilePath srcFile = srcDir + "test.html";
	FilePath cpyFile = cpyDir + "test.html";
	FileSystem::Instance()->CopyFile(srcFile, cpyFile);
	String url = "file:///" + cpyFile.GetAbsolutePathname();

	webView2 = new UIWebView(Rect(30, 150, 200, 100));
	webView2->SetVisible(true);
	webView2->SetBackgroundTransparency(true);
	webView2->OpenURL(url);
	AddControl(webView2);

	Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);
	font->SetSize(20);

	testButton = new UIButton(Rect(0, 510, 300, 30));
	testButton->SetStateFont(0xFF, font);
	testButton->SetStateText(0xFF, L"Finish Test");
	testButton->SetStateFontColor(0xFF, Color::White());
	testButton->SetDebugDraw(true);
	testButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &TransparentWebViewTest::ButtonPressed));
	AddControl(testButton);

	SafeRelease(font);
}

void TransparentWebViewTest::UnloadResources()
{
	RemoveAllControls();

	SafeRelease(webView1);
	SafeRelease(webView2);
	SafeRelease(testButton);
}

bool TransparentWebViewTest::RunTest(int32 testNum)
{
	TestTemplate<TransparentWebViewTest>::RunTest(testNum);
	return testFinished;
}

void TransparentWebViewTest::DidAppear()
{
	onScreenTime = 0.f;
}

void TransparentWebViewTest::Update(float32 timeElapsed)
{
	onScreenTime += timeElapsed;
	if(onScreenTime > TRANSPARENT_WEB_VIEW_TEST_AUTO_CLOSE_TIME)
	{
		testFinished = true;
	}

	TestTemplate<TransparentWebViewTest>::Update(timeElapsed);
}

void TransparentWebViewTest::TestFunction(PerfFuncData * data)
{
	return;
}

void TransparentWebViewTest::ButtonPressed(BaseObject *obj, void *data, void *callerData)
{
	testFinished = true;
}
