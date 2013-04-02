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
//#include "Base/Introspection.h"

namespace DAVA
{
/**
	\ingroup filesystem
	\brief class to work with file pathname
 */
class FilePath
{
public:

	FilePath();
    FilePath(const FilePath & path);
    FilePath(const String & sourcePath);
    FilePath(const String & directory, const String & filename);

	virtual ~FilePath();

    /**
        \brief Function to retrive FilePath with new extension without changing of source FilePath object
        \param[in] pathname is source FilePath object
        \param[in] extension is new extension
        \returns resolved FilePath object with new extension
	 */
    static FilePath CreateWithNewExtension(const FilePath &pathname, const String &extension);


    FilePath& operator=(const FilePath & path);
    FilePath operator+(const FilePath & path) const;
    FilePath& operator+=(const FilePath & path);
    bool operator==(const FilePath & path) const;
	bool operator!=(const FilePath & path) const;

	/*
        \brief Function to check is filepath empty or no
        \returns true if absolutePathname is not empty
	 */
    const bool IsInitalized() const;

	/*
        \brief Function to check is filepath represent folder path
        \returns true if absolutePathname has '/' as last charachter
	 */
    const bool IsDirectoryPathname() const;

	/**
        \brief Function to retrieve pathname
        \returns pathname value
	 */
    const String & GetAbsolutePathname() const;
    
	/**
        \brief Function to retrieve filename from pathname. Filename for path "/Users/Folder/image.png" is "image.png".
        \returns filename value
	 */
    String GetFilename() const;

    /**
        \brief Function to retrieve basename from pathname. Basename for path "/Users/Folder/image.png" is "image".
        \returns basename value
	 */
    String GetBasename() const;
    
	/**
        \brief Function to retrieve extension from pathname. Extension for path "/Users/Folder/image.png" is ".png".
        \returns extension value
	 */
    String GetExtension() const;

	/**
        \brief Function to retrieve directory from pathname. Directory for path "/Users/Folder/image.png" is "/Users/Folder/".
        \returns directory value
	 */
    String GetDirectory() const;
    
    
	/**
        \brief Function to retrieve relative pathname for current working directory
        \returns relative path value
	 */
    String GetRelativePathname() const;

	/**
        \brief Function to retrieve relative pathname for exact directory
        \param[in] forDirectory is exact directory for relative path calculation
        \returns relative path value
	 */
    String GetRelativePathname(const String &forDirectory) const;
    String GetRelativePathname(const FilePath &forDirectory) const;
    
    
	/**
        \brief Function for replacement of original filename
        \param[in] filename is new filename
	 */
    void ReplaceFilename(const String &filename);

	/**
        \brief Function for replacement of original basename
        \param[in] basename is new basename
	 */
    void ReplaceBasename(const String &basename);

	/**
        \brief Function for replacement of original extension
        \param[in] extension is new extension
	 */
    void ReplaceExtension(const String &extension);
    
	/**
        \brief Function for replacement of original directory
        \param[in] directory is new directory
	 */
    void ReplaceDirectory(const String &directory);
    void ReplaceDirectory(const FilePath &directory);
    
	/**
        \brief Function for setup of project path for resolving pathnames such as "~res:/Gfx/image.png"
        \param[in] pathname is project path
	 */
    static void SetProjectPathname(const String &pathname);

	/**
        \brief Function to retrive project path for resolving pathnames such as "~res:/Gfx/image.png"
        \returns project path 
	 */
    static const String & GetProjectPathname();

    
	/**
        \brief Function to retrive system path for resolving pathnames such as "~res:/Gfx/image.png", "~doc:/Project/cache.dat"
        \returns resolved pathname in system style. For example "~doc:/Project/cache.dat" will be resolved as "/User/Documents/Project/cache.dat"
	 */
    String ResolvePathname() const;

    
	/**
        \brief Function to modify absolute to be path fo folder. For example "Users/Document" after function call will be "Users/Document/"
	 */
    void MakeDirectoryPathname();

	/**
        \brief Function to truncate extension from path
	 */
    void TruncateExtension();
    
    /**
        \brief Function to retrive last directory name from FilePath that represents directory pathname
        \returns last directory name
	 */
    String GetLastDirectoryName() const;
    
    
	/**
        \brief Function for replacement of pathname from absolutepath
        \param[in] pathname is pathname for replacement
	 */
    void ReplacePath(const FilePath &pathname);
    
    
protected:
    
    static String NormalizePathname(const String &pathname);
    static String MakeDirectory(const String &pathname);

    static String AbsoluteToRelative(const String &directoryPathname, const String &absolutePathname);

    static String GetFilename(const String &pathname);
    static String GetDirectory(const String &pathname);

    static String GetSystemPathname(const String &pathname);
    
    static bool IsAbsolutePathname(const String &pathname);
    
protected:
    
    String absolutePathname;
    static String projectPathname;
    
public:
    
//    INTROSPECTION(FilePath,
//        MEMBER(absolutePathname, "absolutePathname", INTROSPECTION_EDITOR)
// 		NULL
//     );
    
};
};

#endif //__DAVAENGINE_FILE_PATH_H__