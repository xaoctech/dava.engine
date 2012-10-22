#ifndef __QT_UTILS_H__
#define __QT_UTILS_H__

#include "DAVAEngine.h"
#include <QString>

#define QSTRING_TO_DAVASTRING(str)   (str).toStdString().data()

DAVA::String PathnameToDAVAStyle(const DAVA::String &convertedPathname);
DAVA::String PathnameToDAVAStyle(const QString &convertedPathname);


DAVA::String GetOpenFileName(const DAVA::String &title, const DAVA::String &pathname, const DAVA::String &filter);


DAVA::WideString SizeInBytesToWideString(DAVA::float32 size);
DAVA::String SizeInBytesToString(DAVA::float32 size);


DAVA::String GetTextureFileExtensions();


#endif // __QT_UTILS_H__
