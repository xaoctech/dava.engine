/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "TextureDescriptorUtils.h"

void TextureDescriptorUtils::ResaveDescriptors(const FilePath &folderPathname)
{
	FileList * fileList = new FileList(folderPathname);
    if(!fileList) return;
    
	for (int32 fi = 0; fi < fileList->GetCount(); ++fi)
	{
		if (fileList->IsDirectory(fi))
		{
            String name = fileList->GetFilename(fi);
            
            if(0 != CompareCaseInsensitive(String(".svn"), name) && !fileList->IsNavigationDirectory(fi))
            {
                ResaveDescriptors(fileList->GetPathname(fi));
            }
		}
        else if(fileList->GetPathname(fi).IsEqualToExtension(TextureDescriptor::GetDescriptorExtension()))
        {
            TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(fileList->GetPathname(fi));
            descriptor->Save();
            SafeRelease(descriptor);
        }
	}
    
	SafeRelease(fileList);
}


void TextureDescriptorUtils::CopyCompressionParams(const FilePath &folderPathname)
{
	FileList * fileList = new FileList(folderPathname);
    if(!fileList) return;
    
	for (int32 fi = 0; fi < fileList->GetCount(); ++fi)
	{
		if (fileList->IsDirectory(fi))
		{
            String name = fileList->GetFilename(fi);
            
            if(0 != CompareCaseInsensitive(String(".svn"), name) && !fileList->IsNavigationDirectory(fi))
            {
                CopyCompressionParams(fileList->GetPathname(fi));
            }
		}
        else if(fileList->GetPathname(fi).IsEqualToExtension(TextureDescriptor::GetDescriptorExtension()))
        {
            CopyCompression(fileList->GetPathname(fi));
        }
	}
    
	SafeRelease(fileList);
}

void TextureDescriptorUtils::CopyCompression(const FilePath &descriptorPathname)
{
    TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(descriptorPathname);
    if(!descriptor) return;

    const TextureDescriptor::Compression &srcCompression = descriptor->compression[GPU_POWERVR_IOS];
    if(srcCompression.format == FORMAT_INVALID)
    {   //src format not set
        SafeRelease(descriptor);
        return;
    }
    
    for(int32 gpu = GPU_POWERVR_ANDROID; gpu < GPU_FAMILY_COUNT; ++gpu)
    {
        if(descriptor->compression[gpu].format != FORMAT_INVALID)
            continue;
        
        descriptor->compression[gpu].compressToWidth = srcCompression.compressToWidth;
        descriptor->compression[gpu].compressToHeight = srcCompression.compressToHeight;
        descriptor->compression[gpu].sourceFileCrc = srcCompression.sourceFileCrc;
        
        if((srcCompression.format == FORMAT_PVR2 || srcCompression.format == FORMAT_PVR4) && (gpu != GPU_POWERVR_ANDROID))
        {
            descriptor->compression[gpu].format = FORMAT_ETC1;
        }
        else
        {
            descriptor->compression[gpu].format = srcCompression.format;
        }
    }
    
    descriptor->Save();
    SafeRelease(descriptor);
}


void TextureDescriptorUtils::CreateDescriptors(const FilePath &folderPathname)
{
	FileList * fileList = new FileList(folderPathname);
    if(!fileList) return;
    
	for (int32 fi = 0; fi < fileList->GetCount(); ++fi)
	{
		if (fileList->IsDirectory(fi))
		{
            String name = fileList->GetFilename(fi);
            if(0 != CompareCaseInsensitive(String(".svn"), name) && !fileList->IsNavigationDirectory(fi))
            {
                CreateDescriptors(fileList->GetPathname(fi));
            }
		}
        else if(fileList->GetPathname(fi).IsEqualToExtension(".png"))
        {
            CreateDescriptorIfNeed(fileList->GetPathname(fi));
        }
	}
    
	SafeRelease(fileList);
}


void TextureDescriptorUtils::CreateDescriptorIfNeed(const FilePath &pngPathname)
{
    FilePath descriptorPathname = TextureDescriptor::GetDescriptorPathname(pngPathname);
    if(false == FileSystem::Instance()->IsFile(descriptorPathname))
    {
        TextureDescriptor *descriptor = new TextureDescriptor();
        descriptor->Save(descriptorPathname);
        SafeRelease(descriptor);
    }
}

void TextureDescriptorUtils::SetCompressionParams( const FilePath &filePathname, const DAVA::Map<DAVA::eGPUFamily, DAVA::TextureDescriptor::Compression> & compressionParams, bool force)
{
	TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(filePathname);
	if(!descriptor) return;

	auto endIt = compressionParams.end();
	for(auto it = compressionParams.begin(); it != endIt; ++it)
	{
		eGPUFamily gpu = it->first;
		const TextureDescriptor::Compression & compression = it->second;

		if(force || descriptor->compression[gpu].format == FORMAT_INVALID)
		{
			descriptor->compression[gpu] = compression;
		}
	}

	descriptor->Save();
	SafeRelease(descriptor);
}

