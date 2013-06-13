#ifndef __FILE_COMMANDS_H__
#define __FILE_COMMANDS_H__

#include "Command.h"
#include "../Constants.h"
#include "EditorScene.h"

class CommandOpenScene: public Command
{
public:	
	DAVA_DEPRECATED(CommandOpenScene(const DAVA::FilePath &scenePathname = DAVA::FilePath())); // DEPRECATED: using QFileDialog
    
protected:	
    
    virtual void Execute();
    
protected:
    
    DAVA::FilePath selectedScenePathname;
};


class CommandSaveSpecifiedScene: public Command
{
public:	
	DAVA_DEPRECATED(CommandSaveSpecifiedScene(Entity *activeScene, FilePath& filePath)); // DEPRECATED: using QFileDialog
    
protected:	
    
    virtual void Execute();

	Entity*		activeScene;
	FilePath	filePath;
};

class CommandExport: public Command
{
    
public:	
	DAVA_DEPRECATED(CommandExport(DAVA::ImageFileFormat fmt)); // DEPRECATED: using of SceneEditorScreenMain, Cancel absent
    
protected:	
    
    virtual void Execute();
    
protected:
    DAVA::ImageFileFormat format;
    
};

class CommandSaveToFolderWithChilds: public Command
{
public:
	DAVA_DEPRECATED(CommandSaveToFolderWithChilds()); // DEPRECATED: using QFileDialog
protected:
        virtual void Execute();
};



#endif // #ifndef __FILE_COMMANDS_H__