#include "ResultScreen.h"
#include "SettingsManager.h"
#include "GameCore.h"
#include "Config.h"
#include "DeviceInfo.h"

ResultScreen::ResultScreen(const LandscapeTestData& testData, const FilePath& filename, Texture* landscapeTexture)
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
	resultSprite = Sprite::CreateAsRenderTarget(spriteSize.x * RESULT_TEXTURE_SCALE,
                                                spriteSize.y * RESULT_TEXTURE_SCALE,
                                                FORMAT_RGBA8888, true);
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

void ResultScreen::SaveResults()
{
    Core *core=DAVA::Core::Instance();
    Vector2 screenSize(core->GetVirtualScreenWidth(), core->GetVirtualScreenHeight());
    
    Image* image = resultSprite->GetTexture()->CreateImageFromMemory();
    FilePath saveFileName = FileSystem::Instance()->GetUserDocumentsPath();
    saveFileName += FilePath(filename.GetAbsolutePathname() + ".png");
    ImageLoader::Save(image, saveFileName);
    
    Map<String, String> results;
    results["TextureMemorySize"] = Format("%.2f Mb", testData.GetTextureMemorySize()/(1024.f * 1024.f));
    results["SceneFileSize"] = Format("%.2f Mb", testData.GetSceneFileSize()/(1024.f * 1024.f));
    
    
    FilePath documentsPath("~doc:");
    FilePath folderPathname = documentsPath + FilePath("PerformanceTestResult/");
    FileSystem::Instance()->CreateDirectory(folderPathname);
    FilePath statFileName = folderPathname + filename + FilePath(".txt");
    File* file = File::Create(statFileName, File::CREATE | File::WRITE);
    if (file)
    {
        Map<String, String>::const_iterator it = results.begin();
        for(; it != results.end(); it++)
        {
            // "Format" sometimes doesn't work correct with strings on some platforms
            file->WriteLine(((*it).first + ": " + (*it).second).c_str());
        }
        
        SafeRelease(file);
    }
    
    FilePath levelName = FilePath::CreateWithNewExtension(filename, "");
    GameCore::Instance()->FlushToDB(levelName, results, saveFileName);
    
    state = RESULT_STATE_FINISHED;
}

void ResultScreen::Input(UIEvent * event)
{
	if(event->phase == UIEvent::PHASE_BEGAN)
    {
		if(!isFinished && state == RESULT_STATE_NORMAL)
        {
            if(resultSprite != 0)
            {
                SaveResults();
            }
		}
	}
}

void ResultScreen::Update(float32 timeElapsed)
{
	UIScreen::Update(timeElapsed);
	
    if(!isFinished && state == RESULT_STATE_NORMAL && resultSprite != 0)
    {
        SaveResults();
    }
    
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
    textureSprite->SetScale(RESULT_TEXTURE_SCALE, RESULT_TEXTURE_SCALE);
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
			helper->FillRect(Rect(pos - Vector2(MINFPS_TARGET_SIZE, MINFPS_TARGET_SIZE),
                                  Vector2(MINFPS_TARGET_SIZE * 2, MINFPS_TARGET_SIZE * 2)));

#if defined(__DAVAENGINE_OPENGL__)
			glLineWidth(MINFPS_TARGET_SIZE);
#endif
			helper->DrawLine(pos, target);
#if defined(__DAVAENGINE_OPENGL__)
			glLineWidth(MINFPS_TARGET_SIZE);
#endif
		}
		else
		{
			break;
		}
	}
}
