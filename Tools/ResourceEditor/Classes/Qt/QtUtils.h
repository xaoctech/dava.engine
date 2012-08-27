#ifndef __QT_UTILS_H__
#define __QT_UTILS_H__

#include "DAVAEngine.h"
#include <QString>

#define QSTRING_TO_DAVASTRING(str)   (str).toStdString().data()

DAVA::String PathnameToDAVAStyle(const DAVA::String &convertedPathname);
DAVA::String PathnameToDAVAStyle(const QString &convertedPathname);



#endif // __QT_UTILS_H__
