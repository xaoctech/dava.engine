/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTR ACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Ivan "Dizz" Petrochenko
=====================================================================================*/

#include "PVRTest.h"

PVRTest::PVRTest()
: TestTemplate<PVRTest>("PVRTest")
{
    pngSprite = NULL;
    pvrSprite = NULL;

    currentTest = FIRST_TEST;
    for(int32 i = 0; i < TESTS_COUNT + 1; ++i)
    {
        RegisterFunction(this, &PVRTest::TestFunction, Format("PVRTest::TestFunction [%d]", i), NULL);
    }
    
}

void PVRTest::LoadResources()
{
    GetBackground()->SetColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
    
    format = new UIStaticText(Rect(0, 256, 512, 30));
    format->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    
    Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    font->SetSize(20);
    font->SetColor(Color::Black());
    format->SetFont(font);
    SafeRelease(font);
        
    AddControl(format);
}

void PVRTest::ReloadSprites()
{
    Logger::Debug("Test numer: %d", currentTest);
    
    SafeRelease(pngSprite);
    SafeRelease(pvrSprite);
    
    Image::EnableAlphaPremultiplication(false);

    pngSprite = CreateSpriteFromTexture(String(Format("~res:/PVRTest/PNG/number_%d.png", currentTest)));
    pvrSprite = CreateSpriteFromTexture(String(Format("~res:/PVRTest/PVR/number_%d.pvr", currentTest)));
    
    Image::EnableAlphaPremultiplication(true);
    
    static const WideString formats[] =
    {
        L"PVR 2",
        L"PVR 4",
        L"RGBA 8888",
        L"RGBA 4444",
        L"RGBA 5551",
        L"RGB 888",
        L"RGB 565",
        L"A8",
        L"",
        L"",
        L"",
        L"",
        L"",
        L"",
        L""
    };
    
    format->SetText(formats[currentTest]);
}

Sprite * PVRTest::CreateSpriteFromTexture(const String &texturePathname)
{
    Sprite *createdSprite = NULL;
    
    
    Texture *texture = Texture::CreateFromFile(texturePathname);
    if(texture)
    {
        createdSprite = Sprite::CreateFromTexture(texture, 0, 0, texture->GetWidth(), texture->GetHeight());
        texture->Release();
    }
    
    return createdSprite;
}


void PVRTest::UnloadResources()
{
    RemoveAllControls();
    
    SafeRelease(format);
    
    SafeRelease(pngSprite);
    SafeRelease(pvrSprite);
}

void PVRTest::TestFunction(PerfFuncData * data)
{
#if defined (__DAVAENGINE_IPHONE__)
    int32 COUNTER = 1000;
#else //#if defined (__DAVAENGINE_IPHONE__)
    int32 COUNTER = 5000;
#endif //#if defined (__DAVAENGINE_IPHONE__)
    
    for(int32 i = 0; i < COUNTER; ++i)
    {
        double c = 0;
        for(int32 j = 0; j < 60000; ++j)
        {
            c = i * 10 + 5;
        }
        int32 a = c;
    }

    ReloadSprites();
    ++currentTest;
}

void PVRTest::Draw(const DAVA::UIGeometricData &geometricData)
{
    RenderManager::Instance()->ClearWithColor(0.f, 0.7f, 0.f, 1.f);
    
    RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
    
    if(pngSprite)
    {
        pngSprite->SetPosition(0, 0);
        pngSprite->SetScaleSize(256.f, 256.f);
        pngSprite->Draw();
    }
    
    if(pvrSprite)
    {
        pvrSprite->SetPosition(260.f, 0);
        pvrSprite->SetScaleSize(256.f, 256.f);
        pvrSprite->Draw();
    }
    
    TestTemplate<PVRTest>::Draw(geometricData);
}
