#ifndef ZIPHELPER_H
#define ZIPHELPER_H

#include <QString>

class zipHelper
{
public:
    static bool unZipFile(const QString& archiveFilePath, const QString& extDirPath);
};

#endif // ZIPHELPER_H
