//
//  StringUtils.h
//  UIEditor
//
//  Created by Yuri Coder on 11/16/12.
//
//

#ifndef __UIEditor__StringUtils__
#define __UIEditor__StringUtils__

#include "DAVAEngine.h"
#include <QString>

namespace DAVA {
    
// Different string utilities.
// Truncate the file extension.
QString TruncateFileExtension(const QString& fileName, const QString& extension);

// Truncate the ".txt" file extension.
QString TruncateTxtFileExtension(const QString& fileName);

//convert QString to WideString
WideString QStrint2WideString(const QString& str);

//convert WideString to QString
QString WideString2QStrint(const WideString& str);

};

#endif /* defined(__UIEditor__Utils__) */
