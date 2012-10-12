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
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "Render/Texture.h"
#include "Render/LibPVRHelper.h"
#include "Platform/SystemTimer.h"
#include "FileSystem/File.h"
#include "Render/RenderManager.h"

#include "Render/TextureDescriptor.h"

namespace DAVA
{

Texture * Texture::CreateFromPVR(const String & pathName, TextureDescriptor *descriptor)
{
	uint64 timeCreateFromPVR = SystemTimer::Instance()->AbsoluteMS();
    
	File * fp = File::Create(pathName, File::OPEN|File::READ);
	if (!fp)
	{
		Logger::Error("Failed to open PVR texture: %s", pathName.c_str());
		return 0;
	}
	uint32 fileSize = fp->GetSize();
	uint8 * bytes = new uint8[fileSize];
	uint32 dataRead = fp->Read(bytes, fileSize);
    
	if (dataRead != fileSize)
	{
		Logger::Error("Failed to read data from PVR texture file: %s", pathName.c_str());
		return 0;
	}

	Texture * newTexture = UnpackPVRData(bytes, fileSize);
	if (!newTexture)
	{
		Logger::Error("Failed to parse PVR texture: %s", pathName.c_str());
    }

	SafeDeleteArray(bytes);
	SafeRelease(fp);
    
    RenderManager::Instance()->LockNonMain();
#if defined(__DAVAENGINE_OPENGL__)
	int32 saveId = RenderManager::Instance()->HWglGetLastTextureID();
	RenderManager::Instance()->HWglBindTexture(newTexture->id);
    
	if (descriptor->GenerateMipMaps())
	{
        RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
        RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
    else
	{
		RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	
	if (saveId != 0)
	{
		RenderManager::Instance()->HWglBindTexture(saveId);
	}

    
#endif //#if defined(__DAVAENGINE_OPENGL__)
    RenderManager::Instance()->UnlockNonMain();

	timeCreateFromPVR = SystemTimer::Instance()->AbsoluteMS() - timeCreateFromPVR;
	Logger::Debug("TexturePVR: creation time: %lld", timeCreateFromPVR);
    
	return newTexture;
}
    
Texture * Texture::UnpackPVRData(uint8 * data, uint32 dataSize)
{
    bool preloaded = LibPVRHelper::PreparePVRData((const char *)data, dataSize);
    if(!preloaded)
    {
        Logger::Error("[Texture::UnpackPVRData]: can't preload pvr texture");
        return NULL;
    }
    
    Texture * texture = new Texture();
    
    bool filled = LibPVRHelper::FillTextureWithPVRData((const char *)data, dataSize, texture);
    if(!filled)
    {
        SafeRelease(texture);
    }
    return texture;
}


};

