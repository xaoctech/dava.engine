#include "ResultScreen.h"
#include "SettingsManager.h"

ResultScreen::ResultScreen(const Vector<float32>* const fpsStatistics, const String& filename, int32 testCount, int32 testNumber)
:   resultSprite(0)
{
	data[0] = fpsStatistics[0];
	data[1] = fpsStatistics[1];
	data[2] = fpsStatistics[2];
	
	this->filename = filename;
	this->testCount = testCount;
	this->testNumber = testNumber;
	
	
	isFinished = false;
	state = RESULT_STATE_NORMAL;
	
	FTFont *font=FTFont::Create("~res:/Fonts/Ubuntu-R.ttf");
	font->SetColor(1.f, 1.f, 1.f, 1.f);
	font->SetSize(22.f);
	
	fileNameText = new UIStaticText(Rect(0, 0, 0, 0), false);
	fileNameText->SetFont(font);
	fileNameText->SetAlign(ALIGN_TOP | ALIGN_HCENTER);
	AddControl(fileNameText);
	
	screenshotText = new UIStaticText(Rect(0, 0, 0, 0), false);
	screenshotText->SetFont(font);
	screenshotText->SetAlign(ALIGN_TOP | ALIGN_LEFT);
	screenshotText->SetMultiline(true);
	AddControl(screenshotText);
	
	for(int32 i = 0; i < 3; ++i)
    {
		statText[i] = new UIStaticText(Rect(0, 0, 0, 0), false);
		statText[i]->SetFont(font);
		statText[i]->SetAlign(ALIGN_TOP | ALIGN_HCENTER);
		AddControl(statText[i]);
	}
	
	tapToContinue = new UIStaticText(Rect(0, 0, 0, 0), false);
	tapToContinue->SetFont(font);
	tapToContinue->SetAlign(ALIGN_BOTTOM | ALIGN_HCENTER);
	AddControl(tapToContinue);
}

ResultScreen::~ResultScreen()
{
    SafeRelease(fileNameText);
    SafeRelease(statText[0]);
    SafeRelease(statText[1]);
    SafeRelease(statText[2]);
    SafeRelease(tapToContinue);
    SafeRelease(screenshotText);
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
    PrepareSprite();
}

void ResultScreen::WillDisappear()
{
    if(resultSprite)
        SafeRelease(resultSprite);
}

void ResultScreen::Input(UIEvent * event)
{
	if(event->phase == UIEvent::PHASE_BEGAN)
    {
		if(!isFinished && state == RESULT_STATE_NORMAL)
        {
            if(resultSprite != 0)
            {
                Core *core=DAVA::Core::Instance();
                Vector2 screenSize(core->GetVirtualScreenWidth(), core->GetVirtualScreenHeight());

                Image* image = resultSprite->GetTexture()->CreateImageFromMemory();
                image->Resize((int32)screenSize.x, (int32)screenSize.y);

#ifdef __DAVAENGINE_IPHONE__
                image->SaveToSystemPhotos(this);
                state = RESULT_STATE_MAKING_SCREEN_SHOT;
#else
                String saveFileName = FileSystem::Instance()->GetUserDocumentsPath();
                saveFileName += filename + ".png";
                image->Save(saveFileName);
                state = RESULT_STATE_FINISHED;
#endif
            }
		}
	}
}

#ifdef __DAVAENGINE_IPHONE__
void ResultScreen::SaveToSystemPhotosFinished()
{
    state = RESULT_STATE_FINISHED;
}
#endif

void ResultScreen::Update(float32 timeElapsed)
{
	UIScreen::Update(timeElapsed);
	
	switch (state)
    {
		case RESULT_STATE_MAKING_SCREEN_SHOT:
            break;
			
		case RESULT_STATE_FINISHED:
			isFinished=true;
			break;
			
		default:
			break;
	}
}

void ResultScreen::SystemDraw(const UIGeometricData &geometricData)
{
    DVASSERT(resultSprite != 0);

    //render whole screen to sprite
    RenderManager::Instance()->SetRenderTarget(resultSprite);
    RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 1.f);
    UIScreen::SystemDraw(geometricData);
    RenderManager::Instance()->RestoreRenderTarget();

    //draw prepared sprite
    resultSprite->Draw();
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

	for(int32 i = 0; i < 3; ++i)
    {
		Rect r;
		r.SetPosition(Vector2(1.f, i * imageCellSize.y + 1));
		r.SetSize(imageFieldSize);
		DrawStatImage(data + i, r);
		
		Rect textR;
		textR.SetPosition(Vector2(0.f, r.y + r.dy));
		textR.SetSize(textFieldSize);
		statText[i]->SetRect(textR);
		statText[i]->SetText(texts[i]);
	}
	
	Rect fileNameTextRect;
	fileNameTextRect.SetPosition(Vector2(imageCellSize.x, 1.f));
	fileNameTextRect.SetSize(Vector2(screenSize.x - imageCellSize.x, 100.f));

	fileNameText->SetRect(fileNameTextRect);
	fileNameText->SetText(StringToWString(filename));

	Rect tapToContinueRect;
	tapToContinueRect.SetSize(Vector2(screenSize.x - imageCellSize.x, 150.f));
	tapToContinueRect.SetPosition(Vector2(imageCellSize.x, screenSize.y - tapToContinueRect.GetSize().y));

	tapToContinue->SetRect(tapToContinueRect);
	tapToContinue->SetText(L"tap to make screenshot and continue");

    UIScreen::Draw(geometricData);
}

void ResultScreen::PrepareSprite()
{
    SafeRelease(resultSprite);

	Core *core=DAVA::Core::Instance();
    Vector2 screenSize(core->GetVirtualScreenWidth(), core->GetVirtualScreenHeight());
    resultSprite = Sprite::CreateAsRenderTarget(screenSize.x, screenSize.y, FORMAT_RGBA8888);
    
    return;
}

void ResultScreen::DrawStatImage(Vector<float32> *v, Rect rect)
{
	RenderHelper *helper = RenderHelper::Instance();
	RenderManager *manager = RenderManager::Instance();
	
	SettingsManager *settings = SettingsManager::Instance();
	int32 partX = settings->GetLandscapePartitioning().x;
	int32 partY = settings->GetLandscapePartitioning().y;

	Vector2 barSize;
	barSize.Set((rect.dx - partX) / partX, (rect.dy - partY) / partY);
	
	int32 x = 0;
	int32 xDir = 1;
	int32 n = 0;
	for(int32 y = 0; y < partY; ++y) {
		Rect curRect;
		for(int32 i = partX; i; --i, x += xDir)
        {
			Vector2 pos;
			pos.Set(x * (barSize.x + 1), (partY - 1 - y) * (barSize.y + 1));
			curRect.SetPosition(pos + rect.GetPosition());
			curRect.SetSize(barSize);
			
			manager->SetColor(PickColor((*v)[n]));
			helper->FillRect(curRect);
			++n;
		}
		x -= xDir;
		xDir = -xDir;
	}
}

Color ResultScreen::PickColor(float32 fps) const
{
	const float32 fpsList[] =
    {
		-std::numeric_limits<float32>::infinity(), 0.f, 15.f, 20.f, 35.f, 45.f, std::numeric_limits<float32>::infinity()
	};
	const Color colors[sizeof(fpsList) - 1] =
    {
		Color(0xff, 0xff, 0xff, 255.f) / 255.f,
		Color(0xff, 0x00, 0x00, 255.f) / 255.f,
		Color(0xff, 0xff, 0x3e, 255.f) / 255.f,
		Color(0x92, 0xd0, 0x50, 255.f) / 255.f,
		Color(0x00, 0xb0, 0x50, 255.f) / 255.f,
		Color(0x00, 0x70, 0xc0, 255.f) / 255.f
	};
	
	for(int32 i = 0; i < sizeof(fpsList) - 1; ++i)
    {
		if(fps >= fpsList[i] && fps < fpsList[i + 1])
        {
			return colors[i];
		}
	}
	return colors[0];
}