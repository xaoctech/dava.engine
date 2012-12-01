#include "FileCommands.h"

#include "DAVAEngine.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/EditorSettings.h"
#include "../SceneEditor/EditorConfig.h"
#include "../SceneEditor/SceneValidator.h"

#include "../Qt/Main/QtUtils.h"
#include "../Qt/Main/GUIState.h"
#include "../Qt/Main/QtMainWindowHandler.h"
#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"

#include <QFileDialog>
#include <QString>

using namespace DAVA;

//Open Project
CommandOpenProject::CommandOpenProject()
    :   Command(Command::COMMAND_CLEAR_UNDO_QUEUE)
{
}


void CommandOpenProject::Execute()
{
    QString path = QFileDialog::getExistingDirectory(NULL, QString("Open Project Folder"), QString("/"));
    
    if(0 < path.size())
    {
		String projectPath = PathnameToDAVAStyle(path);
		if('/' != projectPath[projectPath.length() - 1])
        {
            projectPath += '/';
        }
        
        EditorSettings::Instance()->SetProjectPath(projectPath);
        String dataSource3Dpathname = projectPath + String("DataSource/3d/");
        EditorSettings::Instance()->SetDataSourcePath(dataSource3Dpathname);
		EditorSettings::Instance()->Save();

        SceneValidator::Instance()->CreateDefaultDescriptors(dataSource3Dpathname);
		SceneValidator::Instance()->SetPathForChecking(projectPath);

		EditorConfig::Instance()->ParseConfig(projectPath + "EditorConfig.yaml");
		
		SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
        if(screen)
        {
            screen->UpdateModificationPanel();
		}
		
		SceneData *activeScene = SceneDataManager::Instance()->GetActiveScene();
        if(activeScene)
        {
            activeScene->ReloadLibrary();
        }
	}

	QtMainWindowHandler::Instance()->RestoreDefaultFocus();
}


//Open scene
CommandOpenScene::CommandOpenScene(const DAVA::String &scenePathname/* = DAVA::String("") */)
    :   Command(Command::COMMAND_CLEAR_UNDO_QUEUE)
    ,   selectedScenePathname(scenePathname)
{
}


void CommandOpenScene::Execute()
{
    if(0 == selectedScenePathname.length())
    {
        String dataSourcePath = EditorSettings::Instance()->GetDataSourcePath();
        selectedScenePathname = GetOpenFileName(String("Open Scene File"), (dataSourcePath.c_str()), String("Scene File (*.sc2)"));
    }
    
    if(0 < selectedScenePathname.size())
    {
        SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
        if(screen)
        {
            screen->NewScene();
            
            EditorSettings::Instance()->AddLastOpenedFile(selectedScenePathname);
            screen->OpenFileAtScene(selectedScenePathname);
            
            GUIState::Instance()->SetNeedUpdatedFileMenu(true);
        }
        
        QtMainWindowHandler::Instance()->ShowStatusBarMessage(selectedScenePathname);
    }
}

//New
CommandNewScene::CommandNewScene()
:   Command(Command::COMMAND_CLEAR_UNDO_QUEUE)
{
}


void CommandNewScene::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->NewScene();
        SceneValidator::Instance()->EnumerateSceneTextures();
    }
}


//Save
CommandSaveScene::CommandSaveScene()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandSaveScene::Execute()
{
    SceneData *activeScene = SceneDataManager::Instance()->GetActiveScene();
    if(activeScene->CanSaveScene())
    {
        SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
        
		String currentPath;
		if(0 < screen->CurrentScenePathname().length())
		{
			currentPath = screen->CurrentScenePathname();    
		}
		else
		{
			currentPath = EditorSettings::Instance()->GetDataSourcePath();
		}

        QString filePath = QFileDialog::getSaveFileName(NULL, QString("Save Scene File"), QString(currentPath.c_str()),
                                                        QString("Scene File (*.sc2)")
                                                        );
        if(0 < filePath.size())
        {
			String normalizedPathname = PathnameToDAVAStyle(filePath);

            EditorSettings::Instance()->AddLastOpenedFile(normalizedPathname);
            screen->SaveSceneToFile(normalizedPathname);

            GUIState::Instance()->SetNeedUpdatedFileMenu(true);
        }
    }

	QtMainWindowHandler::Instance()->RestoreDefaultFocus();
}

//Export
CommandExport::CommandExport(ResourceEditor::eExportFormat fmt)
    :   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
    ,   format(fmt)
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


//Save to folder with childs
CommandSaveToFolderWithChilds::CommandSaveToFolderWithChilds()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandSaveToFolderWithChilds::Execute()
{
	QString path = QFileDialog::getExistingDirectory(NULL, QString("Open Folder"), QString("/"));
	
    if(0 < path.size())
    {
		String folderPath = PathnameToDAVAStyle(path);
		if('/' != folderPath[folderPath.length() - 1])
        {
            folderPath += '/';
        }

		SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
		if(screen)
		{
			screen->SaveToFolder(folderPath);
		}
	}
}
