#include "ParticleEditorCommands.h"
#include "DAVAEngine.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/EditorSettings.h"
#include "../ParticlesEditor/ParticlesEditorControl.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "../SceneEditor/SceneGraph.h"

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
	DVASSERT(screen);

	String currentPath = screen->GetParticlesEditor()->GetActiveConfigName();
	if(currentPath.empty() || currentPath[0] == '~'/*default config is ~res:*/)
	{
		currentPath = EditorSettings::Instance()->GetDataSourcePath();
	}

	QString filePath = QFileDialog::getOpenFileName(NULL, QString("Open particle effect"), QString(currentPath.c_str()), QString("Effect File (*.yaml)"));

	String selectedPathname = PathnameToDAVAStyle(filePath);

	if(selectedPathname.length() > 0)
	{
		screen->GetParticlesEditor()->LoadFromYaml(selectedPathname);
		screen->FindCurrentBody()->bodyControl->GetSceneGraph()->UpdatePropertyPanel();
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
	DVASSERT(screen);

	String currentPath = screen->GetParticlesEditor()->GetActiveConfigName();
	if(currentPath.empty() || currentPath[0] == '~'/*default config is ~res:*/)
	{
		currentPath = EditorSettings::Instance()->GetDataSourcePath();
	}

	QString filePath = QFileDialog::getSaveFileName(NULL, QString("Save particle effect"), QString(currentPath.c_str()), QString("Effect File (*.yaml)"));
	if(filePath.size() > 0)
	{
		String normalizedPathname = PathnameToDAVAStyle(filePath);
		screen->GetParticlesEditor()->SaveToYaml(normalizedPathname);
	}

	QtMainWindowHandler::Instance()->RestoreDefaultFocus();
}

CommandOpenParticleEditorSprite::CommandOpenParticleEditorSprite()
:   Command(Command::COMMAND_CLEAR_UNDO_QUEUE)
{

}

void CommandOpenParticleEditorSprite::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	DVASSERT(screen);

	String currentPath = screen->GetParticlesEditor()->GetActiveSpriteName();
	if(currentPath.empty() || currentPath[0] == '~'/*default config is ~res:*/)
	{
		currentPath = EditorSettings::Instance()->GetProjectPath();
	}

	QString filePath = QFileDialog::getOpenFileName(NULL, QString("Open sprite"), QString(currentPath.c_str()), QString("Sprite (*.txt)"));

	String selectedPathname = PathnameToDAVAStyle(filePath);

	if(selectedPathname.length() > 0)
	{
		uint32 pos = selectedPathname.find(".txt");
		selectedPathname = selectedPathname.substr(0, pos);
		String relativePath = "~res:/" + FileSystem::Instance()->AbsoluteToRelativePath(EditorSettings::Instance()->GetProjectPath()+"Data/", selectedPathname);
		screen->GetParticlesEditor()->SetActiveSprite(relativePath);
	}

	QtMainWindowHandler::Instance()->RestoreDefaultFocus();
}
