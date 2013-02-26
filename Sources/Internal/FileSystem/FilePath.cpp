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
#include "FileSystem/FilePath.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{
	
FilePath::FilePath()
{
    sourcePathname = String("");
    absolutePathname = String("");
    relativePathname = String("");
    relativeFolder = String("");
}

FilePath::FilePath(const FilePath &path)
{
    sourcePathname = path.sourcePathname;
    absolutePathname = path.absolutePathname;
    relativePathname = path.relativePathname;
    relativeFolder = path.relativeFolder;
}
    
//FilePath::FilePath(const String &sourcePath)
//{
//	InitFromPathname(sourcePath);
//}
    
FilePath::~FilePath()
{
    
}


    
//void FilePath::InitFromPathname(const String &sourcePath)
//{
//    InitFromAbsolutePath(sourcePath);
////    sourcePathname = sourcePath;
////    absolutePathname = FileSystem::Instance()->SystemPathForFrameworkPath(sourcePath);
////
////	relativePathname = String("");
////	relativeFolder = String("");
//}
    
void FilePath::InitFromAbsolutePath(const String &absolutePath)
{
    sourcePathname = absolutePath;
    absolutePathname = absolutePath;//FileSystem::Instance()->SystemPathForFrameworkPath(absolutePath);
	
	relativePathname = String("");
	relativeFolder = String("");
}
    
void FilePath::InitFromRelativePath(const String &relativePath)
{
    return InitFromRelativePath(relativePath, FileSystem::Instance()->GetCurrentWorkingDirectory());
}
    
void FilePath::InitFromRelativePath(const String &relativePath, const String &folder)
{
    sourcePathname = folder + relativePath;
    SetRelativePath(relativePath, folder);
}
    
	
void FilePath::SetSourcePath(const String &sourcePath)
{
    sourcePathname = sourcePath;
}

void FilePath::SetAbsolutePath(const String &absolutePath)
{
    absolutePathname = absolutePath;
}

void FilePath::SetRelativePath(const String &relativePath)
{
    SetRelativePath(relativeFolder, FileSystem::Instance()->GetCurrentWorkingDirectory());
}

void FilePath::SetRelativePath(const String &relativePath, const String &folder)
{
    relativePathname = relativePath;
    relativeFolder = FileSystem::Instance()->GetCanonicalPath(folder);
    
    absolutePathname = CreateAbsoluteFromRelative(relativePathname, relativeFolder);
}
    
FilePath& FilePath::operator=(const FilePath &path)
{
    this->sourcePathname = path.sourcePathname;
    this->absolutePathname = path.absolutePathname;
    this->relativePathname = path.relativePathname;
    this->relativeFolder = path.relativeFolder;

    return *this;
}
    
//FilePath& FilePath::operator=(const String &pathname)
//{
//	this->InitFromPathname(pathname);
//
//    return *this;
//}
//
//    
//FilePath::operator String()
//{
//    return GetSourcePath();
//}

const String & FilePath::GetSourcePath() const
{
    return sourcePathname;
}

const String & FilePath::GetAbsolutePath() const
{
    return absolutePathname;
}

const String FilePath::GetRelativePath() const
{
    return GetRelativePath(FileSystem::Instance()->GetCurrentWorkingDirectory());
}

const String FilePath::GetRelativePath(const String &folder) const
{
    String path = FileSystem::Instance()->AbsoluteToRelativePath(folder, absolutePathname);
    return path;
}

const String FilePath::CreateAbsoluteFromRelative(const String &relativePath, const String &folder)
{
    String path = folder + String("/") + relativePath;
    return FileSystem::Instance()->GetCanonicalPath(path);
}
    
const bool FilePath::Initalized() const
{
    return (!sourcePathname.empty());
}

const String FilePath::GetExtension() const
{
    String ext = FileSystem::Instance()->GetExtension(absolutePathname);
    return ext;
}

    
}
