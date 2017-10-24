#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
/**
	\ingroup filesystem
	\brief FileSystemDelegate is a class that allows to control execution of FileSystem operations
*/

class FileSystemDelegate
{
public:
    virtual ~FileSystemDelegate() = default;

    /**
        \brief Function to control execution of FileSystem::IsFile
        \param[in] absoluteFilePath resolved absolute path for the file we want to check
        \returns true if delegate allow to check existence of file
    */
    virtual bool IsFileExists(const String& absoluteFilePath) const = 0;

    /**
        \brief Function to control execution of FileSystem::IsDirectory
        \param[in] absoluteFilePath resolved absolute path for the directory we want to check
        \returns true if delegate allow to check existence of directory
     */
    virtual bool IsDirectoryExists(const String& absoluteDirectoryPath) const = 0;

    /**
        \brief Function to control execution of File::Create
        \param[in] absoluteFilePath resolved absolute path for the file we want to create
        \param[in] attributes combinations of File::eFileAttributes
        \returns true if delegate allow to create file with given attribures
     */
    virtual bool CanCreateFile(const String& absoluteFilePath, uint32 attributes) const = 0;
};
}
