#ifndef __LIBRARY_COMMANDS_H__
#define __LIBRARY_COMMANDS_H__

#include "Command.h"
#include "../Constants.h"

class LibraryCommand: public Command
{
public:
    LibraryCommand(const DAVA::String &pathname, eCommandType _type);
    
protected:
    
    bool CheckExtension(const DAVA::String &extenstionToChecking);
    
protected:
    
    DAVA::String filePathname;
};


class CommandAddScene: public LibraryCommand
{
public:	
	CommandAddScene(const DAVA::String &pathname);

protected:	
    
    virtual void Execute();
    virtual void Cancel();
};


class CommandEditScene: public LibraryCommand
{
public:
	CommandEditScene(const DAVA::String &pathname);
    
protected:
    
    virtual void Execute();
};

class CommandReloadScene: public LibraryCommand
{
public:
	CommandReloadScene(const DAVA::String &pathname);
    
protected:
    
    virtual void Execute();
    virtual void Cancel();
};

class CommandReloadEntityFrom: public LibraryCommand
{
public:
	CommandReloadEntityFrom(const DAVA::String &pathname);
    
protected:
    
    virtual void Execute();
    virtual void Cancel();
    
protected:
    
    DAVA::String fromPathname;
};

class CommandConvertScene: public LibraryCommand
{
public:
	CommandConvertScene(const DAVA::String &pathname);
    
protected:
    
    virtual void Execute();
};



#endif // #ifndef __LIBRARY_COMMANDS_H__