#ifndef __COMMAND_OPEN_PROJECT_H__
#define __COMMAND_OPEN_PROJECT_H__

#include "Command.h"
#include "../Constants.h"

class CommandOpenProject: public Command
{
    
public:	
	CommandOpenProject();
	virtual ~CommandOpenProject();

protected:	
    
    virtual void Execute();
};


class CommandOpenScene: public Command
{
    
public:	
	CommandOpenScene(const DAVA::String &scenePathname = DAVA::String(""));
	virtual ~CommandOpenScene();
    
protected:	
    
    virtual void Execute();
    
protected:
    
    DAVA::String selectedScenePathname;
};

class CommandNewScene: public Command
{
    
public:	
	CommandNewScene();
	virtual ~CommandNewScene();
    
protected:	
    
    virtual void Execute();
};

class CommandSaveScene: public Command
{
    
public:	
	CommandSaveScene();
	virtual ~CommandSaveScene();
    
protected:	
    
    virtual void Execute();
};

class CommandExport: public Command
{
    
public:	
	CommandExport(ResourceEditor::eExportFormat fmt);
	virtual ~CommandExport();
    
protected:	
    
    virtual void Execute();
    
protected:
    ResourceEditor::eExportFormat format;
    
};




#endif // #ifndef __COMMAND_H__