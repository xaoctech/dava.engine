#include "QtUtils.h"

#include <QFileDialog>
#include "QtMainWindowHandler.h"

using namespace DAVA;


DAVA::String PathnameToDAVAStyle(const DAVA::String &convertedPathname)
{
	String normalizedPathname = FileSystem::Instance()->NormalizePath(convertedPathname);

	String::size_type colonPos = normalizedPathname.find(":");
	if((String::npos != colonPos) && (colonPos < normalizedPathname.length() - 1))
	{
		normalizedPathname = normalizedPathname.substr(colonPos + 1);
	}

	return normalizedPathname;
}

DAVA::String PathnameToDAVAStyle(const QString &convertedPathname)
{
	return PathnameToDAVAStyle((const String &)QSTRING_TO_DAVASTRING(convertedPathname));
}


DAVA::String GetOpenFileName(const DAVA::String &title, const DAVA::String &pathname, const DAVA::String &filter)
{
    QString filePath = QFileDialog::getOpenFileName(NULL, QString(title.c_str()), QString(pathname.c_str()),
                                                    QString(filter.c_str()));
    
    QtMainWindowHandler::Instance()->RestoreDefaultFocus();

    return PathnameToDAVAStyle(filePath);
}


DAVA::String SizeInBytesToString(DAVA::float32 size)
{
    DAVA::String retString = "";
    
    if(1000000 < size)
    {
        retString = Format("%0.2f MB", size / (1024 * 1024) );
    }
    else if(1000 < size)
    {
        retString = Format("%0.2f KB", size / 1024);
    }
    else
    {
        retString = Format("%d B", (int32)size);
    }
    
    return  retString;
}

DAVA::WideString SizeInBytesToWideString(DAVA::float32 size)
{
    return StringToWString(SizeInBytesToString(size));
}
