#include "Command.h"

using namespace DAVA;


Command::Command(eCommandType _type)
    :   BaseObject()
    ,   commandType(_type)
    ,   commandState(STATE_VALID)
{
    RegisterPointerType<Command *>(String("Command *"));
}

Command::~Command()
{
	
}


String Command::NormalizePath(const String &pathname)
{
	String normalizedPathname = FileSystem::Instance()->NormalizePath(pathname);

	String::size_type colonPos = normalizedPathname.find(":");
	if((String::npos != colonPos) && (colonPos < normalizedPathname.length() - 1))
	{
		normalizedPathname = normalizedPathname.substr(colonPos + 1);
	}

	return normalizedPathname;
}




