//
//  StringUtils.cpp
//  UIEditor
//
//  Created by Yuri Coder on 11/16/12.
//
//

#include "StringUtils.h"

namespace DAVA {

// Truncate the file extension.
QString TruncateFileExtension(const QString& fileName, const QString& extension)
{
    // Just wrap around the particular DAVA engine functions.
    
    String truncatedName = fileName.toStdString();
    
    int truncatedStringLen = truncatedName.length() - extension.length();
    bool endsWithExtension = false;
    if (fileName.length() >= extension.length())
    {
        endsWithExtension = (truncatedName.compare(truncatedStringLen, extension.length(), extension.toStdString()) == 0);
    }
    
    if (endsWithExtension)
    {
        truncatedName.resize(truncatedStringLen);
    }
    
    return QString::fromStdString(truncatedName);
}

QString TruncateTxtFileExtension(const QString& fileName)
{
    FilePath path(fileName.toStdString());
    path.TruncateExtension();
    
    return QString::fromStdString(path.GetAbsolutePathname());
}

WideString QStrint2WideString(const QString& str)
{
#ifdef __DAVAENGINE_MACOS__
	return str.toStdWString();
#else
	return WideString((wchar_t*)str.unicode(), str.length());
#endif
}

QString WideString2QStrint(const WideString& str)
{
#ifdef __DAVAENGINE_MACOS__
	return QString::fromStdWString(str);
#else
	return QString((const QChar*)str.c_str(), str.length());
#endif
}

}
