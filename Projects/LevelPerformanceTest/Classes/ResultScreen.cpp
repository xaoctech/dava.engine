#include "ResultScreen.h"
#include "SettingsManager.h"

ResultScreen::ResultScreen(const LandscapeTestData& testData, const String& filename, Texture* landscapeTexture)
:	isFinished(false),
	state(RESULT_STATE_NORMAL),
	testData(testData)
{
	this->filename = filename;
	
	texture = SafeRetain(landscapeTexture);
	textureSprite = NULL;
	resultSprite = NULL;
}

ResultScreen::~ResultScreen()
{
    SafeRelease(fileNameText);
    SafeRelease(statText[0]);
    SafeRelease(statText[1]);
    SafeRelease(statText[2]);
    SafeRelease(tapToContinue);
    SafeRelease(screenshotText);
	SafeRelease(textureSprite);
	SafeRelease(resultSprite);
	SafeRelease(texture);
}

void ResultScreen::LoadResources()
{
	Vector2 spriteSize((float32)texture->GetWidth(), (float32)texture->GetHeight());
	textureSprite = Sprite::CreateFromTexture(texture, 0, 0, spriteSize.x, spriteSize.y);
	resultSprite = Sprite::CreateAsRenderTarget(spriteSize.x, spriteSize.y, FORMAT_RGBA8888, true);
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
                String saveFileName = FileSystem::Instance()->GetUserDocumentsPath();
                saveFileName += filename + ".png";
                ImageLoader::Save(image, saveFileName);

				//TODO: discuss where exaclty store these results.
				// Currenty they are stored in a plain text file in user documents dir
				String documentsPath = FileSystem::Instance()->SystemPathForFrameworkPath("~doc:");
				String folderPathname = documentsPath + "PerformanceTestResult";
				FileSystem::Instance()->CreateDirectory(folderPathname);
				String statFileName = folderPathname + "/" + filename + ".txt";
				File* file = File::Create(statFileName, File::CREATE | File::WRITE);
				if (file)
				{
					file->WriteLine(Format("Texture memory size: %d", testData.GetTextureMemorySize()));
					file->WriteLine(Format("Scene file size: %d", testData.GetSceneFileSize()));
					SafeRelease(file);
				}

                state = RESULT_STATE_FINISHED;
            }
		}
	}
}

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

void ResultScreen::Draw(const UIGeometricData &geometricData)
{
	Core *core=DAVA::Core::Instance();

	Vector2 screenSize(core->GetVirtualScreenWidth(), core->GetVirtualScreenHeight());
	
	float32 drawSize = Min(screenSize.x, screenSize.y);
	float32 scale = Min(drawSize / resultSprite->GetWidth(), drawSize / resultSprite->GetHeight());
	
	resultSprite->SetPosition(0, 0);
	resultSprite->SetScale(scale, scale);
	
	resultSprite->Draw();

    UIScreen::Draw(geometricData);
}

void ResultScreen::PrepareSprite()
{
	Rect r(0, 0, resultSprite->GetWidth(), resultSprite->GetHeight());

	RenderManager::Instance()->SetRenderTarget(resultSprite);
	textureSprite->Draw();
	DrawStatImage(r);
	DrawMinFpsTargets(r);
	RenderManager::Instance()->RestoreRenderTarget();
}

void ResultScreen::DrawStatImage(Rect rect)
{
	RenderHelper *helper = RenderHelper::Instance();
	RenderManager *manager = RenderManager::Instance();

	for(uint32 i = 0; i < testData.GetItemCount(); ++i)
	{
		FpsStatItem item = testData.GetItem(i);
		manager->SetColor(SettingsManager::Instance()->GetColorByFps(item.minFps));
		helper->FillRect(testData.TranslateRect(item.rect, rect));
	}
}

void ResultScreen::DrawMinFpsTargets(DAVA::Rect rect)
{
	RenderHelper *helper = RenderHelper::Instance();
	RenderManager *manager = RenderManager::Instance();

	uint32 cnt = Min(SettingsManager::Instance()->GetMinFpsSectorCount(), testData.GetItemCount());
	for(uint32 i = 0; i < cnt; ++i)
	{
		FpsStatItem item = testData.GetItem(i);
		if(item.minFps < SettingsManager::Instance()->GetMinFps())
		{
			Vector2 pos(item.position.x, item.position.y);
			Vector2 target(item.viewTarget.x, item.viewTarget.y);

			pos = testData.TranslatePoint(pos, rect);
			target = testData.TranslatePoint(target, rect) - pos;
			target *= 20.f;
			target += pos;

			manager->SetColor(1.f, 1.f, 1.f, 1.f);
			helper->FillRect(Rect(pos - Vector2(10.f, 10.f), Vector2(20.f, 20.f)));

#if defined(__DAVAENGINE_OPENGL__)
			glLineWidth(10.f);
#endif
			helper->DrawLine(pos, target);
#if defined(__DAVAENGINE_OPENGL__)
			glLineWidth(1.f);
#endif
		}
		else
		{
			break;
		}
	}
}
