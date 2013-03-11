#include <QFileDialog>
#include "Project/ProjectManager.h"
#include "Main/QtUtils.h"
#include "Classes/SceneEditor/EditorSettings.h"
#include "Classes/SceneEditor/SceneValidator.h"
#include "Classes/SceneEditor/EditorConfig.h"
#include "Classes/SceneEditor/SceneEditorScreenMain.h"

ProjectManager::ProjectManager()
	: curProjectPath("")
{

}

ProjectManager::~ProjectManager()
{

}

bool ProjectManager::IsOpened()
{
	return (curProjectPath != "");
}

QString ProjectManager::CurProjectPath()
{
	return curProjectPath;
}

QString ProjectManager::CurProjectDataSourcePath()
{
	return QString(EditorSettings::Instance()->GetDataSourcePath().c_str());
}

QString ProjectManager::ProjectOpenDialog()
{
	return QFileDialog::getExistingDirectory(NULL, QString("Open Project Folder"), QString("/"));
}

void ProjectManager::ProjectOpen(const QString &path)
{
	if(path != curProjectPath)
	{
		ProjectClose();

		curProjectPath = path;

		if(!curProjectPath.isEmpty())
		{
			DAVA::String projectPath = PathnameToDAVAStyle(path);
			if('/' != projectPath[projectPath.length() - 1])
			{
				projectPath += '/';
			}

			DAVA::String dataSource3Dpathname = projectPath + DAVA::String("DataSource/3d/");

			EditorSettings::Instance()->SetProjectPath(projectPath);
			EditorSettings::Instance()->SetDataSourcePath(dataSource3Dpathname);
			EditorSettings::Instance()->Save();

			SceneValidator::Instance()->CreateDefaultDescriptors(dataSource3Dpathname);
			SceneValidator::Instance()->SetPathForChecking(projectPath);

			EditorConfig::Instance()->ParseConfig(projectPath + "EditorConfig.yaml");
		}

		SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
		if(screen)
		{
			screen->UpdateModificationPanel();
		}

		emit ProjectOpened(curProjectPath);
	}
}

void ProjectManager::ProjectClose()
{
	if("" != curProjectPath)
	{
		emit ProjectClosed(curProjectPath);
		curProjectPath = "";
	}
}
