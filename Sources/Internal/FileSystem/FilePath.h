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
#ifndef __DAVAENGINE_FILE_PATH_H__
#define __DAVAENGINE_FILE_PATH_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"

namespace DAVA
{
/**
	\ingroup filesystem
	\brief class to work with file pathname
 */
class FilePath: public BaseObject
{
public:
	/*
	
	TODO: new interface

	FilePath();
	FilePath(const FilePath &path);
	FilePath(const char* path);
	FilePath(const String &path);
	virtual ~FilePath();

	FilePath& operator=(const FilePath &path);
	String& operator()(const FilePath &path);

	String AbsolutePath();
	String RelativePath(const FilePath &path = String());
	String AbsoluteDir();
	String RelativeDir(const FilePath &path = String());
	String Filename();
	String Basename();
	String Extension();
	*/	


	FilePath();
    FilePath(const FilePath &path);
//    FilePath(const String &sourcePath); 
	virtual ~FilePath();

    
//    void InitFromPathname(const String &sourcePath);
    void InitFromAbsolutePath(const String &absolutePath);
    void InitFromRelativePath(const String &relativePath);
    void InitFromRelativePath(const String &relativePath, const String &folder);
    
    void SetSourcePath(const String &sourcePath);
    void SetAbsolutePath(const String &absolutePath);
    void SetRelativePath(const String &relativePath);
    void SetRelativePath(const String &relativePath, const String &folder);

    FilePath& operator=(const FilePath &path);
//    FilePath& operator=(const String &pathname);

//	operator String();

    const String & GetSourcePath() const;
    const String & GetAbsolutePath() const;
    const String GetRelativePath() const;
    const String GetRelativePath(const String &folder) const;
    
    const String GetExtension() const;
    
    const bool Initalized() const;
    
protected:
    
    static const String CreateAbsoluteFromRelative(const String &relativePath, const String &folder);
    
protected:

    String sourcePathname;
    String absolutePathname;
    
    String relativePathname;
    String relativeFolder;

public:
    INTROSPECTION_EXTEND(FilePath, BaseObject,
        MEMBER(sourcePathname, "Source Pathname", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(absolutePathname, "Absolute Pathname", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
                         
        MEMBER(relativePathname, "Relative Pathname", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(relativeFolder, "Relative Folder", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
    );
};
};

#endif //__DAVAENGINE_FILE_PATH_H__