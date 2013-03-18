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
#include "Utils/Utils.h"

namespace DAVA
{

String FilePath::projectPathname = String();

void FilePath::SetProjectPathname(const String &pathname)
{
    projectPathname = NormalizePathname(pathname);
}

String & FilePath::GetProjectPathname()
{
    return projectPathname;
}
    
FilePath::FilePath()
{
    absolutePathname = String();
}

FilePath::FilePath(const FilePath &path)
{
    absolutePathname = path.absolutePathname;
}
    
FilePath::FilePath(const String &pathname)
{
    absolutePathname = NormalizePathname(pathname);
}
    
FilePath::FilePath(const String &directory, const String &filename)
{
    absolutePathname = NormalizePathname(directory + filename);
}
    
FilePath::~FilePath()
{
    
}

FilePath& FilePath::operator=(const FilePath &path)
{
    this->absolutePathname = path.absolutePathname;
    return *this;
}
    
FilePath FilePath::operator+(const FilePath &path)
{
    DVASSERT(IsDirectoryPathname());
    
    FilePath pathname(*this);
    pathname.absolutePathname = pathname.absolutePathname + path.absolutePathname;
    
    return pathname;
}
    
    
const bool FilePath::Initalized() const
{
    return (!absolutePathname.empty());
}

const bool FilePath::IsDirectoryPathname() const
{
    if(!Initalized())
    {
        return false;
    }

    const int32 lastPosition = absolutePathname.length() - 1;
    return (absolutePathname.at(lastPosition) == '/');
}


const String FilePath::GetFilename() const
{
    return GetFilename(absolutePathname);
}
    
const String FilePath::GetFilename(const String &pathname)
{
    String::size_type dotpos = pathname.rfind(String("/"));
    if (dotpos == String::npos)
        return String();
    
    return pathname.substr(dotpos+1);
}



const String FilePath::GetBasename() const
{
    const String filename = GetFilename();
    
    const String::size_type dotpos = filename.rfind(String("."));
	if (dotpos == String::npos)
		return filename;
    
	return filename.substr(0, dotpos);
}

const String FilePath::GetExtension() const
{
    const String filename = GetFilename();
    
    const String::size_type dotpos = filename.rfind(String("."));
	if (dotpos == String::npos)
        return String();
    
    return filename.substr(dotpos);
}

    
const String FilePath::GetDirectory() const
{
    return GetDirectory(absolutePathname);
}

const String FilePath::GetDirectory(const String &pathname)
{
    const String::size_type dotpos = pathname.rfind(String("/"));
    if (dotpos == String::npos)
        return String();
    
    return pathname.substr(0, dotpos + 1);
}

    
const String FilePath::GetRelativePathname() const
{
    return GetRelativePathname(FileSystem::Instance()->GetCurrentWorkingDirectory());
}
    
const String FilePath::GetRelativePathname(const String &forDirectory) const
{
    return AbsoluteToRelative(forDirectory, absolutePathname);
}
    
    
void FilePath::ReplaceFilename(const String &filename)
{
    const String directory = GetDirectory();
    absolutePathname = NormalizePathname(directory + filename);
}
    
void FilePath::ReplaceBasename(const String &basename)
{
    const String directory = GetDirectory();
    const String extension = GetExtension();
    absolutePathname = NormalizePathname(directory + basename + extension);
}
    
void FilePath::ReplaceExtension(const String &extension)
{
    const String directory = GetDirectory();
    const String basename = GetBasename();
    absolutePathname = NormalizePathname(directory + basename + extension);
    
}
    
void FilePath::ReplaceDirectory(const String &directory)
{
    const String filename = GetFilename();
    absolutePathname = NormalizePathname(directory + filename);
}

    
const String FilePath::NormalizePathname(const String &pathname)
{
	if(pathname.empty())
		return String();
	
	String path = pathname;
    std::replace(path.begin(), path.end(),'\\','/');
    
    Vector<String> tokens;
    Split(path, "/", tokens);
    
    //TODO: correctly process situation ../../folders/filename
    for (int32 i = 0; i < (int32)tokens.size(); ++i)
    {
        if (String(".") == tokens[i])
        {
            for (int32 k = i + 1; k < (int32)tokens.size(); ++k)
            {
                tokens[k - 1] = tokens[k];
            }
            --i;
            tokens.pop_back();
        }
        else if ((1 <= i) && (String("..") == tokens[i] && String("..") != tokens[i-1]))
        {
            for (int32 k = i + 1; k < (int32)tokens.size(); ++k)
            {
                tokens[k - 2] = tokens[k];
            }
            i-=2;
            tokens.pop_back();
            tokens.pop_back();
        }
    }
    
    String result = "";
    if('/' == path[0])
		result = "/";
    
    for (int32 k = 0; k < (int32)tokens.size(); ++k)
    {
        result += tokens[k];
        if (k + 1 != (int32)tokens.size())
            result += String("/");
    }
    
	//process last /
	if(('/' == path[path.length() - 1]) && (path.length() != 1))
		result += String("/");
    
    return result;
}

const String FilePath::MakeDirectory(const String &pathname)
{
    if(pathname.empty())
    {
        return String();
    }
    
    const int32 lastPosition = pathname.length() - 1;
    if(pathname.at(lastPosition) != '/')
    {
        return pathname + String("/");
    }
    
    return pathname;
}
    
const String FilePath::AbsoluteToRelative(const String &directoryPathname, const String &absolutePathname)
{
    String workingDirectoryPath = directoryPathname;
    String workingFilePath = absolutePathname;
    
    std::replace(workingDirectoryPath.begin(),workingDirectoryPath.end(),'\\','/');
    std::replace(workingFilePath.begin(),workingFilePath.end(),'\\','/');
    
    if((absolutePathname.empty()) || ('/' != absolutePathname[0]))
        return absolutePathname;
    
    String filePath;
    String fileName;
    SplitPath(workingFilePath, filePath, fileName);
    
    Vector<String> folders;
    Split(workingDirectoryPath, "/", folders);
    Vector<String> fileFolders;
    Split(filePath, "/", fileFolders);
    
    Vector<String>::size_type equalCount = 0;
    for(; equalCount < folders.size() && equalCount < fileFolders.size(); ++equalCount)
    {
        if(folders[equalCount] != fileFolders[equalCount])
        {
            break;
        }
    }
    
    String retPath = "";
    for(Vector<String>::size_type i = equalCount; i < folders.size(); ++i)
    {
        retPath += "../";
    }
    
    for(Vector<String>::size_type i = equalCount; i < fileFolders.size(); ++i)
    {
        retPath += fileFolders[i] + "/";
    }
    
    return (retPath + fileName);
}
    

void FilePath::SplitPath(const String & filePath, String & path, String & filename)
{
    String fullPath = NormalizePathname(filePath);
    path = GetDirectory(fullPath);
    filename = GetFilename(fullPath);
}
  
    
}
