#ifndef __LIBRARY_COMMANDS_H__
#define __LIBRARY_COMMANDS_H__

#include "Command.h"
#include "../Constants.h"

class LibraryCommand: public Command
{
public:
    LibraryCommand(const DAVA::FilePath &pathname, eCommandType _type, CommandList::eCommandId id);
    
protected:
    
    bool CheckExtension(const DAVA::String &extenstionToChecking);
    
protected:
    
    DAVA::FilePath filePathname;
};


class CommandAddScene: public LibraryCommand
{
public:	
	DAVA_DEPRECATED(CommandAddScene(const DAVA::FilePath &pathname));// DEPRECATED: using of SceneDataManager

protected:	
    
    virtual void Execute();
    virtual void Cancel();
};


class CommandEditScene: public LibraryCommand
{
public:
	DAVA_DEPRECATED(CommandEditScene(const DAVA::FilePath &pathname)); // DEPRECATED : using SceneDataManager(QOBJECT)
    
protected:
    
    virtual void Execute();
};

class CommandReloadScene: public LibraryCommand
{
public:
	DAVA_DEPRECATED(CommandReloadScene(const DAVA::FilePath &pathname)); // DEPRECATED : using SceneDataManager(QOBJECT)
    
protected:
    
    virtual void Execute();
    virtual void Cancel();
};

class CommandReloadEntityFrom: public LibraryCommand
{
public:
	DAVA_DEPRECATED(CommandReloadEntityFrom(const DAVA::FilePath &pathname)); // DEPRECATED : using SceneDataManager(QOBJECT)
    
protected:
    
    virtual void Execute();
    virtual void Cancel();
    
protected:
    
    DAVA::FilePath fromPathname;
};

class CommandConvertScene: public LibraryCommand
{
public:
	CommandConvertScene(const DAVA::FilePath &pathname);
    
protected:
    
    virtual void Execute();
};



#endif // #ifndef __LIBRARY_COMMANDS_H__