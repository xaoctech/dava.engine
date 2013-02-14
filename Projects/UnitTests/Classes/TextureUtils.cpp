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

#include "TextureUtils.h"


Sprite * TextureUtils::CreateSpriteFromTexture(const String &texturePathname)
{
    Sprite *createdSprite = NULL;
    
    Texture *texture = Texture::CreateFromFile(texturePathname);
    if(texture)
    {
        createdSprite = Sprite::CreateFromTexture(texture, 0, 0, (float32)texture->GetWidth(), (float32)texture->GetHeight());
        texture->Release();
    }
    
    return createdSprite;
}

TextureUtils::CompareResult TextureUtils::CompareSprites(Sprite *first, Sprite *second, PixelFormat format)
{
	/*
	DVASSERT(false);
	DebugBreak();
	__debugbreak();
	*/
    DVASSERT(first->GetHeight() == second->GetHeight());
    DVASSERT(first->GetWidth() == second->GetWidth());
    
    Image *firstComparer = CreateImageAsRGBA8888(first);
    Image *secondComparer = CreateImageAsRGBA8888(second);

    CompareResult compareResult = {0};

    
    int32 imageSizeInBytes = (int32)(first->GetWidth() * first->GetHeight() * Texture::GetPixelFormatSizeInBytes(firstComparer->format));

    int32 step = 1;
    int32 startIndex = 0;
    
	if(FORMAT_A8 == format)
	{
		compareResult.bytesCount = (int32)(first->GetWidth() * first->GetHeight() * Texture::GetPixelFormatSizeInBytes(FORMAT_A8));
		step = 4;
		startIndex = 3;
	}
	else
	{
		compareResult.bytesCount = imageSizeInBytes;
	}

	for(int32 i = startIndex; i < imageSizeInBytes; i += step)
	{
		compareResult.difference += abs(firstComparer->GetData()[i] - secondComparer->GetData()[i]);
	}

    
//    String documentsPath = FileSystem::Instance()->GetCurrentDocumentsDirectory();
//    firstComparer->Save(documentsPath + Format("PVRTest/src_number_%d.png", currentTest));
//    secondComparer->Save(documentsPath + Format("PVRTest/dst_number_%d.png", currentTest));
    
    
    SafeRelease(firstComparer);
    SafeRelease(secondComparer);
    return compareResult;
}

Image * TextureUtils::CreateImageAsRGBA8888(Sprite *sprite)
{
    Sprite *renderTarget = Sprite::CreateAsRenderTarget(sprite->GetWidth(), sprite->GetHeight(), FORMAT_RGBA8888);
    RenderManager::Instance()->SetRenderTarget(renderTarget);
    
    sprite->SetPosition(0, 0);
    sprite->Draw();
    
    RenderManager::Instance()->RestoreRenderTarget();
    
    Texture *renderTargetTexture = renderTarget->GetTexture();
    Image *resultImage = renderTargetTexture->CreateImageFromMemory();
    
    SafeRelease(renderTarget);
    return resultImage;
}


