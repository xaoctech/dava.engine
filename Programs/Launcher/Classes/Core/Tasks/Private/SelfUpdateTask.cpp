#include "Core/Tasks/SelfUpdateTask.h"
#include "Core/Tasks/UnzipTask.h"
#include "Core/Tasks/DownloadTask.h"
#include "Core/ApplicationQuitController.h"
#include "Core/ApplicationContext.h"

#include "Utils/FileManager.h"

#include <QApplication>

SelfUpdateTask::SelfUpdateTask(ApplicationContext* appContext, ApplicationQuitController* quitController_, const QString& url_)
    : AsyncChainTask(appContext)
    , quitController(quitController_)
    , url(url_)
{
}

QString SelfUpdateTask::GetDescription() const
{
    return QObject::tr("Starting self update");
}

void SelfUpdateTask::Run()
{
    if (appContext->fileManager.IsInQuarantine())
    {
        SetError(QObject::tr("Launcher is in quarantine and can not be updated! Move it from Downloads folder, please!"));
        emit Finished();
        return;
    }

    QString filePath = appContext->fileManager.GetTempDownloadFilePath(url);
    fileToWrite.setFileName(filePath);
    if (fileToWrite.open(QIODevice::WriteOnly) == false)
    {
        SetError(QObject::tr("Can not create file %1!").arg(filePath));
        emit Finished();
        return;
    }

    QString description = QObject::tr("Loading new launcher");
    std::unique_ptr<BaseTask> task = appContext->CreateTask<DownloadTask>(description, url, &fileToWrite);
    appContext->taskManager.AddTask(std::move(task), notifier);
}

int SelfUpdateTask::GetSubtasksCount() const
{
    return 2;
}

void SelfUpdateTask::OnFinished(const BaseTask* task)
{
    if (task->HasError())
    {
        emit Finished();
        return;
    }
    else
    {
        switch (state)
        {
        case LOADING:
            notifier.IncrementStep();
            OnLoaded(task);
            break;
        case UNPACKING:
            OnUnpacked();
            break;
        default:
            Q_ASSERT(false);
            break;
        }
    }
}

void SelfUpdateTask::OnLoaded(const BaseTask* task)
{
    fileToWrite.close();

    QString filePath = appContext->fileManager.GetTempDownloadFilePath(url);

    if (fileToWrite.size() <= 0)
    {
        SetError(QObject::tr("Failed to write file %1!").arg(filePath));
        emit Finished();
        return;
    }

    state = UNPACKING;

    QString selfUpdateDirPath = appContext->fileManager.GetSelfUpdateTempDirectory();

    std::unique_ptr<BaseTask> zipTask = appContext->CreateTask<UnzipTask>(filePath, selfUpdateDirPath);

    appContext->taskManager.AddTask(std::move(zipTask), notifier);
}

void SelfUpdateTask::OnUnpacked()
{
    FileManager::DeleteDirectory(appContext->fileManager.GetTempDirectory());
    QString tempDir = appContext->fileManager.GetTempDirectory(); //create temp directory

    QString appDirPath = appContext->fileManager.GetLauncherDirectory();
    QString selfUpdateDirPath = appContext->fileManager.GetSelfUpdateTempDirectory();

    //remove old launcher files except download folder, temp folder and update folder
    //replace it with task later
    if (appContext->fileManager.MoveLauncherRecursively(appDirPath, tempDir, this)
        && appContext->fileManager.MoveLauncherRecursively(selfUpdateDirPath, appDirPath, this))
    {
        quitController->RestartApplication();
    }

    emit Finished();
}
