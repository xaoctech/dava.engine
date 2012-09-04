#include "ParticleEditorCommands.h"
#include "DAVAEngine.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/EditorSettings.h"
#include "../ParticlesEditor/ParticlesEditorControl.h"

#include "../Qt/QtUtils.h"
#include "../Qt/GUIState.h"
#include "../Qt/QtMainWindowHandler.h"
#include "../Qt/SceneData.h"
#include "../Qt/SceneDataManager.h"

#include <QFileDialog>
#include <QString>

using namespace DAVA;

CommandOpenParticleEditorConfig::CommandOpenParticleEditorConfig()
:   Command(Command::COMMAND_CLEAR_UNDO_QUEUE)
{

}

void CommandOpenParticleEditorConfig::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		String currentPath = screen->GetParticlesEditor()->GetLastOpenedConfigName();
		if(currentPath.empty())
		{
			currentPath = EditorSettings::Instance()->GetProjectPath();
		}

		QString filePath = QFileDialog::getOpenFileName(NULL, QString("Open particle effect"), QString(currentPath.c_str()), QString("Scene File (*.yaml)"));

		String selectedPathname = PathnameToDAVAStyle(filePath);

		if(selectedPathname.length() > 0)
		{
			SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
			if(screen)
			{
				screen->GetParticlesEditor()->LoadFromYaml(selectedPathname);
			}
		}
	}

	QtMainWindowHandler::Instance()->RestoreDefaultFocus();
}

CommandSaveParticleEditorConfig::CommandSaveParticleEditorConfig()
:   Command(Command::COMMAND_CLEAR_UNDO_QUEUE)
{

}

void CommandSaveParticleEditorConfig::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		String currentPath = screen->GetParticlesEditor()->GetLastOpenedConfigName();
		if(currentPath.empty())
		{
			currentPath = EditorSettings::Instance()->GetProjectPath();
		}

		QString filePath = QFileDialog::getSaveFileName(NULL, QString("Save particle effect"), QString(currentPath.c_str()), QString("Scene File (*.yaml)"));
		//if(0 < filePath.size())
		//{
		//	String normalizedPathname = PathnameToDAVAStyle(filePath);

		//	EditorSettings::Instance()->AddLastOpenedFile(normalizedPathname);
		//	screen->SaveSceneToFile(normalizedPathname);

		//	GUIState::Instance()->SetNeedUpdatedFileMenu(true);
		//}
	}

	QtMainWindowHandler::Instance()->RestoreDefaultFocus();
}
