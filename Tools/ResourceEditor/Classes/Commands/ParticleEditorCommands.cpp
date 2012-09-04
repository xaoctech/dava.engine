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
	String dataSourcePath = EditorSettings::Instance()->GetDataSourcePath();
	QString filePath = QFileDialog::getOpenFileName(NULL, QString("Open Scene File"), QString(dataSourcePath.c_str()),
		QString("Scene File (*.sc2)")
		);

	String selectedPathname = PathnameToDAVAStyle(filePath);

	if(selectedPathname.length() > 0)
	{
		SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
		if(screen)
		{
			screen->GetParticlesEditor()->LoadFromYaml(selectedPathname);
		}
	}

	QtMainWindowHandler::Instance()->RestoreDefaultFocus();
}

