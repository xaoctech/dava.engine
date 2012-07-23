#include "FileCommands.h"

#include "DAVAEngine.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/EditorSettings.h"


#include <QFileDialog>
#include <QString>

using namespace DAVA;

//Open Project
CommandOpenProject::CommandOpenProject()
    :   Command(Command::COMMAND_CLEAR_UNDO_QUEUE)
{
}

CommandOpenProject::~CommandOpenProject()
{
	
}


void CommandOpenProject::Execute()
{
    QString path = QFileDialog::getExistingDirectory(NULL, QString("Open Project Folder"), QString("/"));
    
    if(0 < path.size())
    {
        String projectPath = path.toStdString();
        if('/' != projectPath[projectPath.length() - 1])
        {
            projectPath += '/';
        }
        
        EditorSettings::Instance()->SetProjectPath(projectPath);
        EditorSettings::Instance()->SetDataSourcePath(projectPath + String("/DataSource/3d/"));
    }
}


//Open scene
CommandOpenScene::CommandOpenScene(const DAVA::String &scenePathname/* = DAVA::String("") */)
    :   Command(Command::COMMAND_CLEAR_UNDO_QUEUE)
    ,   selectedScenePathname(scenePathname)
{
}

CommandOpenScene::~CommandOpenScene()
{
	
}


void CommandOpenScene::Execute()
{
    if(0 == selectedScenePathname.length())
    {
        String dataSourcePath = EditorSettings::Instance()->GetDataSourcePath();
        QString filePath = QFileDialog::getOpenFileName(NULL, QString("Open Scene File"), QString(dataSourcePath.c_str()),
                                                        QString("Scene File (*.sc2)")
                                                        );
        
        selectedScenePathname = filePath.toStdString();
    }
    
    if(0 < selectedScenePathname.size())
    {
        SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
        if(screen)
        {
            screen->NewScene();
            
            EditorSettings::Instance()->AddLastOpenedFile(selectedScenePathname);
            screen->OpenFileAtScene(selectedScenePathname);
        }
    }
}

//New
CommandNewScene::CommandNewScene()
:   Command(Command::COMMAND_CLEAR_UNDO_QUEUE)
{
}

CommandNewScene::~CommandNewScene()
{
}


void CommandNewScene::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->NewScene();
    }
}


//Save
CommandSaveScene::CommandSaveScene()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}

CommandSaveScene::~CommandSaveScene()
{
	
}


void CommandSaveScene::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen && screen->SaveIsAvailable())
    {
        String currentPath = FileSystem::Instance()->NormalizePath(screen->CurrentScenePathname());    
        String folderPathname, filename;
        FileSystem::Instance()->SplitPath(currentPath, folderPathname, filename);
        
        QString filePath = QFileDialog::getSaveFileName(NULL, QString("Save Scene File"), QString(folderPathname.c_str()),
                                                        QString("Scene File (*.sc2)")
                                                        );
        if(0 < filePath.size())
        {
            EditorSettings::Instance()->AddLastOpenedFile(filePath.toStdString());
            screen->SaveSceneToFile(filePath.toStdString());
        }
    }
}

//Export
CommandExport::CommandExport(ResourceEditor::eExportFormat fmt)
    :   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
    ,   format(fmt)
{
}

CommandExport::~CommandExport()
{
	
}


void CommandExport::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->ExportAs(format);
    }
}





