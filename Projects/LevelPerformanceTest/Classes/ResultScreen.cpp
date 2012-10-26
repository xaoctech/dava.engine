#include "ResultScreen.h"
#include "Constants.h"
#include "ScreenShotHelper.h"

#define DEF_RECT_STEP_X 50
#define DEF_RECT_STEP_Y 20

ResultScreen::ResultScreen(const Vector<float>* const fpsStatistics, const String& filename, int testCount, int testNumber) {
	data[0]=fpsStatistics[0];
	data[1]=fpsStatistics[1];
	data[2]=fpsStatistics[2];
	
	this->filename=filename;
	this->testCount=testCount;
	this->testNumber=testNumber;
	
	
	isFinished=false;
	state=RESULT_STATE_NORMAL;
	
	FTFont *font=FTFont::Create("~res:/Fonts/Ubuntu-R.ttf");
	font->SetColor(1.f, 1.f, 1.f, 1.f);
	font->SetSize(22.f);
	
	fileNameText=new UIStaticText(Rect(0, 0, 0, 0), false);
	fileNameText->SetFont(font);
	fileNameText->SetAlign(ALIGN_TOP | ALIGN_HCENTER);
	AddControl(fileNameText);
	
	screenshotText=new UIStaticText(Rect(0, 0, 0, 0), false);
	screenshotText->SetFont(font);
	screenshotText->SetAlign(ALIGN_TOP | ALIGN_LEFT);
	screenshotText->SetMultiline(true);
	AddControl(screenshotText);
	
	for(int i=0; i<3; ++i) {
		statText[i]=new UIStaticText(Rect(0, 0, 0, 0), false);
		statText[i]->SetFont(font);
		statText[i]->SetAlign(ALIGN_TOP | ALIGN_HCENTER);
		AddControl(statText[i]);
	}
	
	tapToContinue=new UIStaticText(Rect(0, 0, 0, 0), false);
	tapToContinue->SetFont(font);
	tapToContinue->SetAlign(ALIGN_BOTTOM | ALIGN_HCENTER);
	AddControl(tapToContinue);
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
		if(!isFinished && state == RESULT_STATE_NORMAL) {
			ScreenShotHelper::Instance()->MakeScreenShot();
			state=RESULT_STATE_MAKING_SCREEN_SHOT;
		}		
	}
}

void ResultScreen::Update(float32 timeElapsed)
{
	UIScreen::Update(timeElapsed);
	
	switch (state) {
		case RESULT_STATE_MAKING_SCREEN_SHOT:
			if(ScreenShotHelper::Instance()->IsFinished()) {
				state=RESULT_STATE_FINISHED;
			}
			break;
			
		case RESULT_STATE_FINISHED:
			isFinished=true;
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
	imageCellSize.x/=2.f;
	imageCellSize.y/=3.f;
	
	imageFieldSize=imageCellSize;
	imageFieldSize.y*=0.8f;

	textFieldSize.x=imageFieldSize.x;
	textFieldSize.y=imageCellSize.y-imageFieldSize.y;

	const wchar_t* const texts[]={L"min", L"mid", L"max"};

	for(int i=0; i<3; ++i) {
		Rect r;
		r.SetPosition(Vector2(1.f, i*imageCellSize.y+1));
		r.SetSize(imageFieldSize);
		DrawStatImage(data+i, r);
		
		Rect textR;
		textR.SetPosition(Vector2(0.f, r.y+r.dy));
		textR.SetSize(textFieldSize);
		statText[i]->SetRect(textR);
		statText[i]->SetText(texts[i]);
	}
	
	Rect fileNameTextRect;
	fileNameTextRect.SetPosition(Vector2(imageCellSize.x, 1.f));
	fileNameTextRect.SetSize(Vector2(screenSize.x-imageCellSize.x, 100.f));

	fileNameText->SetRect(fileNameTextRect);
	fileNameText->SetText(StringToWString(filename));
	
	Rect tapToContinueRect;
	tapToContinueRect.SetSize(Vector2(screenSize.x-imageCellSize.x, 150.f));
	tapToContinueRect.SetPosition(Vector2(imageCellSize.x, screenSize.y-tapToContinueRect.GetSize().y));

	tapToContinue->SetRect(tapToContinueRect);
	tapToContinue->SetText(L"tap to make screenshot and continue");

	if(state == RESULT_STATE_FINISHED) {
		String fn=ScreenShotHelper::Instance()->GetFileName();
		
		Rect screenshotTextRect;
		screenshotTextRect.SetSize(Vector2(screenSize.x-imageCellSize.x, 400.f));
		screenshotTextRect.SetPosition(Vector2(imageCellSize.x, screenSize.y/2.f - 200.f));

		screenshotText->SetRect(screenshotTextRect);
		screenshotText->SetText(L"Results saved to: "+StringToWString(fn));
		screenshotText->SetMultiline(true, true);
	}
	
	UIScreen::Draw(geometricData);
}

void ResultScreen::DrawStatImage(Vector<float> *v, Rect rect) {
	RenderHelper *pHelper=RenderHelper::Instance();
	RenderManager *pManager=RenderManager::Instance();
	
	Vector2 barSize;
	barSize.Set((rect.dx-X)/X, (rect.dy-Y)/Y);
	
	int x=0;
	int xDir=1;
	int n=0;
	for(int y=0; y<Y; ++y) {
		Rect curRect;
		for(int i=X; i; --i, x+=xDir) {
			Vector2 pos;
			pos.Set(x*(barSize.x+1), y*(barSize.y+1));
			curRect.SetPosition(pos+rect.GetPosition());
			curRect.SetSize(barSize);
			
			pManager->SetColor(PickColor((*v)[n]));
			pHelper->FillRect(curRect);
			++n;
		}
		x-=xDir;
		xDir=-xDir;
	}
}

Color ResultScreen::PickColor(float fps) const {
	const float fpsList[] = {
		-INFINITY, 0.f, 15.f, 20.f, 35.f, 45.f, INFINITY
	};
	const Color colors[sizeof(fpsList)-1] = {
		Color(0xff, 0xff, 0xff, 255.f)/255.f,
		Color(0xff, 0x00, 0x00, 255.f)/255.f,
		Color(0xff, 0xff, 0x3e, 255.f)/255.f,
		Color(0x92, 0xd0, 0x50, 255.f)/255.f,
		Color(0x00, 0xb0, 0x50, 255.f)/255.f,
		Color(0x00, 0x70, 0xc0, 255.f)/255.f
	};
	
	for(int i=0; i<sizeof(fpsList)-1; ++i) {
		if(fps >= fpsList[i] && fps < fpsList[i+1]) {
			return colors[i];
		}
	}
	return colors[0];
}