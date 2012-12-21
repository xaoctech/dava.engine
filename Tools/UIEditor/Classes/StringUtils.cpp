//
//  StringUtils.cpp
//  UIEditor
//
//  Created by Yuri Coder on 11/16/12.
//
//

#include "StringUtils.h"
#include "Utils/Utils.h"

namespace DAVA {

// Truncate the file extension.
QString TruncateFileExtension(const QString& fileName, const QString& extension)
{
    // Just wrap around the particular DAVA engine functions.
    return QString::fromStdString(TruncateFileExtension(fileName.toStdString(), extension.toStdString()));
}

QString TruncateTxtFileExtension(const QString& fileName)
{
    return QString::fromStdString(TruncateTxtFileExtension(fileName.toStdString()));
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
