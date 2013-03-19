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

	FilePath();
    FilePath(const FilePath &path);
    FilePath(const String &sourcePath);
    FilePath(const String &directory, const String &filename);

	virtual ~FilePath();

    FilePath& operator=(const FilePath &path);
    FilePath operator+(const FilePath &path);
    bool operator==(const FilePath &path) const;
	bool operator!=(const FilePath &path) const;

    
    const bool IsInitalized() const;
    const bool IsDirectoryPathname() const;

    const String GetAbsolutePathname() const;
    const String GetFilename() const;
    const String GetBasename() const;
    const String GetExtension() const;
    const String GetDirectory() const;
    
    const String GetRelativePathname() const;
    const String GetRelativePathname(const String &forDirectory) const;
    
    void ReplaceFilename(const String &filename);
    void ReplaceBasename(const String &basename);
    void ReplaceExtension(const String &extension);
    void ReplaceDirectory(const String &directory);
    
    static void SetProjectPathname(const String &pathname);
    static String & GetProjectPathname();

    const String ResolvePathname() const;
    
protected:
    
    static const String NormalizePathname(const String &pathname);
    static const String MakeDirectory(const String &pathname);

    static const String AbsoluteToRelative(const String &directoryPathname, const String &absolutePathname);

    static const String GetFilename(const String &pathname);
    static const String GetDirectory(const String &pathname);

    static const String GetSystemPathname(const String &pathname);
    
    
protected:
    
    String absolutePathname;
    static String projectPathname;

public:
    INTROSPECTION_EXTEND(FilePath, BaseObject,
        NULL
    );
};
};

#endif //__DAVAENGINE_FILE_PATH_H__