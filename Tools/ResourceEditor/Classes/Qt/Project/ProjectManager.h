#ifndef __PROJECT_MANAGER_H__
#define __PROJECT_MANAGER_H__

#include <QObject>
#include "DAVAEngine.h"

class ProjectManager : public QObject, public DAVA::Singleton<ProjectManager>
{
	Q_OBJECT

public:
	ProjectManager();
	~ProjectManager();

	bool isOpened();

	QString CurProjectPath();
	QString CurProjectDataSourcePath();

public slots:
	QString ProjectOpenDialog();
	void ProjectOpen(const QString &path);
	void ProjectClose();

signals:
	void ProjectOpened(const QString &path);
	void ProjectClosed(const QString &path);

private:
	QString curProjectPath;
};

#endif // __PROJECT_MANAGER_H__ 
