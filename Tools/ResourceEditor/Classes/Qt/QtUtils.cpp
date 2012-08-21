#include "QtUtils.h"

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
