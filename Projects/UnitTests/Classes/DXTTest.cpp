#include "DXTTest.h"
#include "TextureUtils.h"

static const PixelFormat formats[] =
{
	FORMAT_RGBA8888,
	FORMAT_DXT1,
	FORMAT_DXT1NM,
	FORMAT_DXT1A,
	FORMAT_DXT3,
	FORMAT_DXT5,
	FORMAT_DXT5NM
};

DXTTest::DXTTest()
: TestTemplate<DXTTest>("DXTTest")
{
    String testFolder = FileSystem::Instance()->GetCurrentDocumentsDirectory() + String("/DXTTest/");
    FileSystem::Instance()->CreateDirectory(testFolder, true);

    pngSprite = NULL;
    dxtSprite = NULL;
    decompressedPNGSprite = NULL;

    currentTest = FIRST_TEST;
    for(int32 i = 0; i < TESTS_COUNT; ++i)
    {
        PixelFormatDescriptor formatDescriptor = Texture::GetPixelFormatDescriptor(formats[i]);
        RegisterFunction(this, &DXTTest::TestFunction, Format("DXTTest of %s", formatDescriptor.name.c_str()), NULL);
    }
}

void DXTTest::LoadResources()
{
    GetBackground()->SetColor(Color(0.0f, 1.0f, 0.0f, 1.0f));

    Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);

    font->SetSize(20);
    font->SetColor(Color::White());

    compareResultText = new UIStaticText(Rect(0, 256, 512, 200));
    compareResultText->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
    compareResultText->SetMultiline(true);
    compareResultText->SetFont(font);
    AddControl(compareResultText);

    SafeRelease(font);
}


void DXTTest::UnloadResources()
{
    RemoveAllControls();

    SafeRelease(compareResultText);

    SafeRelease(pngSprite);
    SafeRelease(dxtSprite);
    SafeRelease(decompressedPNGSprite);
}

void DXTTest::TestFunction(PerfFuncData * data)
{
    DVASSERT(currentTest < TESTS_COUNT);

    if(IsCurrentTestAccepted())
    {
        ReloadSprites();

		float32 differencePersentage = 100.f;
		if (decompressedPNGSprite->GetSize() == dxtSprite->GetSize())
		{
			TextureUtils::CompareResult result = TextureUtils::CompareSprites(decompressedPNGSprite, dxtSprite, formats[currentTest]);
			differencePersentage = ((float32)result.difference / ((float32)result.bytesCount * 256.f)) * 100.f;
		}
        
        PixelFormatDescriptor formatDescriptor = Texture::GetPixelFormatDescriptor(formats[currentTest]);
        data->testData.message = Format("\nDifference: %f%%\nCoincidence: %f%%",
                                        differencePersentage, 100.f - differencePersentage);

        compareResultText->SetText(StringToWString(data->testData.message));
        Logger::Debug(data->testData.message.c_str());

        TEST_VERIFY(differencePersentage < (float32)ACCETABLE_DELTA_IN_PERSENTS);

        //Save images for visual comparision
        Image *firstComparer = TextureUtils::CreateImageAsRGBA8888(decompressedPNGSprite);
        Image *secondComparer = TextureUtils::CreateImageAsRGBA8888(dxtSprite);
        
        String documentsPath = FileSystem::Instance()->GetCurrentDocumentsDirectory();
        ImageLoader::Save(firstComparer, documentsPath + Format("DXTTest/src_number_%d.png", currentTest));
        ImageLoader::Save(secondComparer, documentsPath + Format("DXTTest/dst_number_%d.png", currentTest));
    }

    ++currentTest;
}

bool DXTTest::IsCurrentTestAccepted()
{
    RenderManager::Caps deviceCaps = RenderManager::Instance()->GetCaps();

    if((formats[currentTest] == FORMAT_RGBA16161616) && !deviceCaps.isFloat16Supported)
    {
        return false;
    }
    
    if((formats[currentTest] == FORMAT_RGBA32323232) && !deviceCaps.isFloat32Supported)
    {
        return false;
    }


	PixelFormatDescriptor pixelFormat = Texture::GetPixelFormatDescriptor(formats[currentTest]);
	
	if (pixelFormat.format == 0)
		return false;
	
	if ((pixelFormat.format == FORMAT_DXT1A || pixelFormat.format == FORMAT_DXT5NM)
		 && (deviceCaps.isDXTSupported && 0 == pixelFormat.internalformat))
	{
		return false;
	}

	if(formats[currentTest] == FORMAT_DXT1A || formats[currentTest] == FORMAT_DXT1NM || formats[currentTest] == FORMAT_DXT5NM)
	{
		//not all DXT formats are supported 
		return false;
	}
	

	
	return true;
}


void DXTTest::ReloadSprites()
{
    SafeRelease(pngSprite);
    SafeRelease(dxtSprite);
    SafeRelease(decompressedPNGSprite);

    pngSprite = TextureUtils::CreateSpriteFromTexture(String(Format("~res:/TestData/DXTTest/PNG/number_%d.png", currentTest)));
    dxtSprite = TextureUtils::CreateSpriteFromTexture(String(Format("~res:/TestData/DXTTest/DXT/number_%d.dds", currentTest)));
    decompressedPNGSprite = TextureUtils::CreateSpriteFromTexture(String(Format("~res:/TestData/DXTTest/DecompressedPNG/number_%d.png", currentTest)));
}

void DXTTest::Draw(const DAVA::UIGeometricData &geometricData)
{
    RenderManager::Instance()->ClearWithColor(0.f, 0.0f, 0.f, 1.f);
    
    RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);

    if(pngSprite)
    {
        pngSprite->SetPosition(0, 0);
        pngSprite->SetScaleSize(256.f, 256.f);
        pngSprite->Draw();
    }

    if(dxtSprite)
    {
        dxtSprite->SetPosition(260.f, 0);
        dxtSprite->SetScaleSize(256.f, 256.f);
        dxtSprite->Draw();
    }

    TestTemplate<DXTTest>::Draw(geometricData);
}


