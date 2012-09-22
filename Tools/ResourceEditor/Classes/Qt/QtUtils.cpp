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
