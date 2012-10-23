#include "ResultScreen.h"
#include "ScreenShotHelper.h"
#include "SettingsManager.h"

ResultScreen::ResultScreen(const Vector<float>* const fpsStatistics, const String& filename, int testCount, int testNumber) {
	m_Data[0] = fpsStatistics[0];
	m_Data[1] = fpsStatistics[1];
	m_Data[2] = fpsStatistics[2];
	
	m_Filename = filename;
	m_nTestCount = testCount;
	m_nTestNumber = testNumber;
	
	
	m_bIsFinished = false;
	m_State = RESULT_STATE_NORMAL;
	
	FTFont *font=FTFont::Create("~res:/Fonts/Ubuntu-R.ttf");
	font->SetColor(1.f, 1.f, 1.f, 1.f);
	font->SetSize(22.f);
	
	m_pFileNameText = new UIStaticText(Rect(0, 0, 0, 0), false);
	m_pFileNameText->SetFont(font);
	m_pFileNameText->SetAlign(ALIGN_TOP | ALIGN_HCENTER);
	AddControl(m_pFileNameText);
	
	m_pScreenshotText = new UIStaticText(Rect(0, 0, 0, 0), false);
	m_pScreenshotText->SetFont(font);
	m_pScreenshotText->SetAlign(ALIGN_TOP | ALIGN_LEFT);
	m_pScreenshotText->SetMultiline(true);
	AddControl(m_pScreenshotText);
	
	for(int i = 0; i < 3; ++i) {
		m_pStatText[i] = new UIStaticText(Rect(0, 0, 0, 0), false);
		m_pStatText[i]->SetFont(font);
		m_pStatText[i]->SetAlign(ALIGN_TOP | ALIGN_HCENTER);
		AddControl(m_pStatText[i]);
	}
	
	m_pTapToContinue = new UIStaticText(Rect(0, 0, 0, 0), false);
	m_pTapToContinue->SetFont(font);
	m_pTapToContinue->SetAlign(ALIGN_BOTTOM | ALIGN_HCENTER);
	AddControl(m_pTapToContinue);
}

void ResultScreen::LoadResources()
{
}

void ResultScreen::UnloadResources()
{
	RemoveAllControls();
}

void ResultScreen::WillAppear()
{
}

void ResultScreen::WillDisappear()
{
}

void ResultScreen::Input(UIEvent * event)
{
	if(event->phase == UIEvent::PHASE_BEGAN) {
		if(!m_bIsFinished && m_State == RESULT_STATE_NORMAL) {
			ScreenShotHelper::Instance()->MakeScreenShot();
			m_State = RESULT_STATE_MAKING_SCREEN_SHOT;
		}		
	}
}

void ResultScreen::Update(float32 timeElapsed)
{
	UIScreen::Update(timeElapsed);
	
	switch (m_State) {
		case RESULT_STATE_MAKING_SCREEN_SHOT:
			if(ScreenShotHelper::Instance()->IsFinished()) {
				m_State=RESULT_STATE_FINISHED;
			}
			break;
			
		case RESULT_STATE_FINISHED:
			m_bIsFinished=true;
			break;
			
		default:
			break;
	}
}

void ResultScreen::Draw(const UIGeometricData &geometricData)
{
	Core *core=DAVA::Core::Instance();

	Vector2 screenSize(core->GetVirtualScreenWidth(), core->GetVirtualScreenHeight());
	
	Vector2 imageCellSize(screenSize);
	Vector2 imageFieldSize;
	Vector2 textFieldSize;
	imageCellSize.x /= 2.f;
	imageCellSize.y /= 3.f;
	
	imageFieldSize = imageCellSize;
	imageFieldSize.y *= 0.8f;

	textFieldSize.x = imageFieldSize.x;
	textFieldSize.y = imageCellSize.y - imageFieldSize.y;

	const wchar_t* const texts[] = {L"min", L"mid", L"max"};

	for(int i = 0; i < 3; ++i) {
		Rect r;
		r.SetPosition(Vector2(1.f, i * imageCellSize.y + 1));
		r.SetSize(imageFieldSize);
		DrawStatImage(m_Data + i, r);
		
		Rect textR;
		textR.SetPosition(Vector2(0.f, r.y + r.dy));
		textR.SetSize(textFieldSize);
		m_pStatText[i]->SetRect(textR);
		m_pStatText[i]->SetText(texts[i]);
	}
	
	Rect fileNameTextRect;
	fileNameTextRect.SetPosition(Vector2(imageCellSize.x, 1.f));
	fileNameTextRect.SetSize(Vector2(screenSize.x - imageCellSize.x, 100.f));

	m_pFileNameText->SetRect(fileNameTextRect);
	m_pFileNameText->SetText(StringToWString(m_Filename));

	Rect tapToContinueRect;
	tapToContinueRect.SetSize(Vector2(screenSize.x - imageCellSize.x, 150.f));
	tapToContinueRect.SetPosition(Vector2(imageCellSize.x, screenSize.y - tapToContinueRect.GetSize().y));

	m_pTapToContinue->SetRect(tapToContinueRect);
	m_pTapToContinue->SetText(L"tap to make screenshot and continue");

	if(m_State == RESULT_STATE_FINISHED) {
		String fn = ScreenShotHelper::Instance()->GetFileName();
		
		Rect screenshotTextRect;
		screenshotTextRect.SetSize(Vector2(screenSize.x - imageCellSize.x, 400.f));
		screenshotTextRect.SetPosition(Vector2(imageCellSize.x, screenSize.y / 2.f - 200.f));

		m_pScreenshotText->SetRect(screenshotTextRect);
		m_pScreenshotText->SetText(L"Results saved to: " + StringToWString(fn));
		m_pScreenshotText->SetMultiline(true, true);
	}

	UIScreen::Draw(geometricData);
}

void ResultScreen::DrawStatImage(Vector<float> *v, Rect rect) {
	RenderHelper *pHelper = RenderHelper::Instance();
	RenderManager *pManager = RenderManager::Instance();
	
	SettingsManager *pSettings = SettingsManager::Instance();
	int X = pSettings->GetLandscapePartitioning().x;
	int Y = pSettings->GetLandscapePartitioning().y;

	Vector2 barSize;
	barSize.Set((rect.dx - X) / X, (rect.dy - Y) / Y);
	
	int x = 0;
	int xDir = 1;
	int n = 0;
	for(int y = 0; y < Y; ++y) {
		Rect curRect;
		for(int i = X; i; --i, x += xDir) {
			Vector2 pos;
			pos.Set(x * (barSize.x + 1), y * (barSize.y + 1));
			curRect.SetPosition(pos + rect.GetPosition());
			curRect.SetSize(barSize);
			
			pManager->SetColor(PickColor((*v)[n]));
			pHelper->FillRect(curRect);
			++n;
		}
		x -= xDir;
		xDir = -xDir;
	}
}

Color ResultScreen::PickColor(float fps) const {
	const float fpsList[] = {
		-INFINITY, 0.f, 15.f, 20.f, 35.f, 45.f, INFINITY
	};
	const Color colors[sizeof(fpsList) - 1] = {
		Color(0xff, 0xff, 0xff, 255.f) / 255.f,
		Color(0xff, 0x00, 0x00, 255.f) / 255.f,
		Color(0xff, 0xff, 0x3e, 255.f) / 255.f,
		Color(0x92, 0xd0, 0x50, 255.f) / 255.f,
		Color(0x00, 0xb0, 0x50, 255.f) / 255.f,
		Color(0x00, 0x70, 0xc0, 255.f) / 255.f
	};
	
	for(int i = 0; i < sizeof(fpsList) - 1; ++i) {
		if(fps >= fpsList[i] && fps < fpsList[i + 1]) {
			return colors[i];
		}
	}
	return colors[0];
}