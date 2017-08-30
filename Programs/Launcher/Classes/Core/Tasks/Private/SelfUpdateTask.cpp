#include "Core/Tasks/SelfUpdateTask.h"
#include "Core/Tasks/UnzipTask.h"
#include "Core/Tasks/DownloadTask.h"

#include "Core/ApplicationManager.h"
#include "Core/ApplicationQuitController.h"

#include "Utils/FileManager.h"

#include <QApplication>

SelfUpdateTask::SelfUpdateTask(ApplicationManager* appManager, ApplicationQuitController* quitController_, const QString& url_)
    : AsyncChainTask(appManager)
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
    FileManager* fileManager = appManager->GetFileManager();
    if (fileManager->IsInQuarantine())
    {
        SetError(QObject::tr("Launcher is in quarantine and can not be updated! Move it from Downloads folder, please!"));
        emit Finished();
        return;
    }

    QString filePath = fileManager->GetTempDownloadFilePath(url);
    fileToWrite.setFileName(filePath);
    if (fileToWrite.open(QIODevice::WriteOnly) == false)
    {
        SetError(QObject::tr("Can not create file %1!").arg(filePath));
        emit Finished();
        return;
    }

    QString description = QObject::tr("Loading new launcher");
    std::unique_ptr<BaseTask> task = appManager->CreateTask<DownloadTask>(description, url, &fileToWrite);
    appManager->AddTaskWithNotifier(std::move(task), notifier);
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

    FileManager* fileManager = appManager->GetFileManager();
    QString filePath = fileManager->GetTempDownloadFilePath(url);

    if (fileToWrite.size() <= 0)
    {
        SetError(QObject::tr("Failed to write file %1!").arg(filePath));
        emit Finished();
        return;
    }

    state = UNPACKING;

    QString selfUpdateDirPath = fileManager->GetSelfUpdateTempDirectory();

    std::unique_ptr<BaseTask> zipTask = appManager->CreateTask<UnzipTask>(filePath, selfUpdateDirPath);

    appManager->AddTaskWithNotifier(std::move(zipTask), notifier);
}

void SelfUpdateTask::OnUnpacked()
{
    FileManager* fileManager = appManager->GetFileManager();
    FileManager::DeleteDirectory(fileManager->GetTempDirectory());
    QString tempDir = fileManager->GetTempDirectory(); //create temp directory

    QString appDirPath = fileManager->GetLauncherDirectory();
    QString selfUpdateDirPath = fileManager->GetSelfUpdateTempDirectory();

    //remove old launcher files except download folder, temp folder and update folder
    //replace it with task later
    if (fileManager->MoveLauncherRecursively(appDirPath, tempDir, this)
        && fileManager->MoveLauncherRecursively(selfUpdateDirPath, appDirPath, this))
    {
        quitController->RestartApplication();
    }

    emit Finished();
}
