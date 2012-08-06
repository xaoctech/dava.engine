#ifndef __FILE_COMMANDS_H__
#define __FILE_COMMANDS_H__

#include "Command.h"
#include "../Constants.h"

class CommandOpenProject: public Command
{
public:	
	CommandOpenProject();

protected:	
    
    virtual void Execute();
};


class CommandOpenScene: public Command
{
public:	
	CommandOpenScene(const DAVA::String &scenePathname = DAVA::String(""));
    
protected:	
    
    virtual void Execute();
    
protected:
    
    DAVA::String selectedScenePathname;
};

class CommandNewScene: public Command
{
public:	
	CommandNewScene();
    
protected:	
    
    virtual void Execute();
};

class CommandSaveScene: public Command
{
public:	
	CommandSaveScene();
    
protected:	
    
    virtual void Execute();
};

class CommandExport: public Command
{
    
public:	
	CommandExport(ResourceEditor::eExportFormat fmt);
    
protected:	
    
    virtual void Execute();
    
protected:
    ResourceEditor::eExportFormat format;
    
};




#endif // #ifndef __FILE_COMMANDS_H__