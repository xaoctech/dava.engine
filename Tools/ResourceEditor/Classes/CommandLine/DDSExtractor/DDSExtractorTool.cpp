/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "DDSExtractorTool.h"

#include "TexturePacker/CommandLineParser.h"

using namespace DAVA;

void DDSExtractorTool::PrintUsage()
{
    printf("\n");
    printf("-extract [-folder [directory] -mipmaps [number]]\n");
    printf("\twill extract mentioned number of mipmaps\n");
    printf("\tfrom all dds files, located in specified folder\n");

    printf("\n");
    printf("Sample:\n");
    printf("-extract -folder /Users/User/Project/Data/3d -mipmaps 2 -forceclose\n");
}

DAVA::String DDSExtractorTool::GetCommandLineKey()
{
    return "-extract";
}

bool DDSExtractorTool::InitializeFromCommandLine()
{
	sourcePath = CommandLineParser::GetCommandParam(String("-folder"));
	
	if(sourcePath.IsEmpty())
	{
		errors.insert(String("Incorrect params for source folder"));
		return false;
	}
	
    sourcePath.MakeDirectoryPathname();
	String mipmapsNumberStr = CommandLineParser::GetCommandParam(String("-mipmaps"));
	
	int32 number = atoi(mipmapsNumberStr.c_str());
	if (number <= 0)
	{
		errors.insert(String("Incorrect param for mipmaps number"));
		return false;
	}
	mipmapsNumber = number;
	
	return true;
}

void DDSExtractorTool::Process()
{
	if (!sourcePath.Exists())
	{
		return;
	}
	DAVA::List<DAVA::FilePath> allFiles = GetFilesFromFolderRecursively(sourcePath);
	DAVA::List<DAVA::FilePath>::const_iterator it = allFiles.begin();
	DAVA::List<DAVA::FilePath>::const_iterator end = allFiles.end();
	for(; it != end; ++it)
	{
		ExtractImagesFromFile(*it);
	}
}

void DDSExtractorTool::ExtractImagesFromFile(const DAVA::FilePath& path)
{
	if(!path.IsEqualToExtension(".dds"))
	{
		return;
	}
	
	Vector<Image *> imageSet = ImageLoader::CreateFromFile(path);
	if(0 == imageSet.size())
	{
		return;
	}
	
	uint32 size = mipmapsNumber >= imageSet.size() ? imageSet.size() : mipmapsNumber;
	for (uint32 i = 0; i < size; ++i)
	{
		FilePath saveFilePath(path);
		saveFilePath.ReplaceExtension(".png");
		saveFilePath.ReplaceBasename(path.GetBasename() + Format("_%u", imageSet[i]->GetHeight()));
		ImageLoader::Save(imageSet[i], saveFilePath);
	}
	for_each(imageSet.begin(), imageSet.end(), SafeRelease<Image>);
}


DAVA::List<DAVA::FilePath> DDSExtractorTool::GetFilesFromFolderRecursively(const DAVA::FilePath& path)
{
	DAVA::List<DAVA::FilePath> retList;
	FileList * fileList = new FileList(path);
	for(uint32 i = 0; i < fileList->GetCount(); ++i)
	{
		if (fileList->IsNavigationDirectory(i))
		{
			continue;
		}
		if(fileList->IsDirectory(i))
		{
			DAVA::List<FilePath> subSet = GetFilesFromFolderRecursively(fileList->GetPathname(i));
			retList.insert(retList.end(), subSet.begin(), subSet.end());
		}
		else
		{
			retList.push_back(fileList->GetPathname(i));
		}
	}
	SafeRelease(fileList);
	return retList;
}

