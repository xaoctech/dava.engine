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
    
}
