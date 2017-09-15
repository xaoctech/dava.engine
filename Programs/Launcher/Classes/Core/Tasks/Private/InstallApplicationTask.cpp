#include "Core/Tasks/InstallApplicationTask.h"
#include "Core/ApplicationContext.h"
#include "Core/Tasks/DownloadTask.h"
#include "Core/Tasks/RemoveApplicationTask.h"
#include "Core/Tasks/UnzipTask.h"
#include "Core/Tasks/RunApplicationTask.h"
#include "Core/ConfigHolder.h"

#include "Utils/AppsCommandsSender.h"
#include "Utils/FileManager.h"
#include "Utils/Utils.h"

#include <QNetworkReply>
#include <QEventLoop>

InstallApplicationTask::InstallApplicationTask(ApplicationContext* appContext, ConfigHolder* configHolder_, const InstallApplicationParams& params_)
    : AsyncChainTask(appContext)
    , params(params_)
    , configHolder(configHolder_)
{
}

QString InstallApplicationTask::GetDescription() const
{
    return QObject::tr("Installing application %1").arg(LauncherUtils::GetAppName(params.app, params.newVersion.isToolSet));
}

void InstallApplicationTask::Run()
{
    //replace this code later with "Create file while loading"
    QString filePath = appContext->fileManager.GetTempDownloadFilePath(params.newVersion.url);
    fileToWrite.setFileName(filePath);
    if (fileToWrite.open(QIODevice::WriteOnly | QIODevice::Truncate) == false)
    {
        SetError(QObject::tr("Can not create file %1!").arg(filePath));
        emit Finished();
        return;
    }

    QString description = QObject::tr("Downloading application %1").arg(LauncherUtils::GetAppName(params.app, params.newVersion.isToolSet));
    std::unique_ptr<BaseTask> task = appContext->CreateTask<DownloadTask>(description, params.newVersion.url, &fileToWrite);

    appContext->taskManager.AddTask(std::move(task), notifier);
}

int InstallApplicationTask::GetSubtasksCount() const
{
    return NeedUnpack() ? 2 : 1;
}

void InstallApplicationTask::OnFinished(const BaseTask* task)
{
    //ignore children run tasks
    if (dynamic_cast<const RunApplicationTask*>(task) != nullptr)
    {
        return;
    }
    if (task->HasError() == false)
    {
        switch (state)
        {
        case LOADING:
            notifier.IncrementStep();
            OnLoaded(task);
            return;
        case UNPACKING:
            Install();
            return;
        case POST_INSTALL:
            OnInstalled();
            return;
        default:
            Q_ASSERT(false);
            break;
        }
    }
    emit Finished();
}

void InstallApplicationTask::OnLoaded(const BaseTask* task)
{
    QString filePath = appContext->fileManager.GetTempDownloadFilePath(params.newVersion.url);

    fileToWrite.close();
    if (fileToWrite.size() <= 0)
    {
        SetError(QObject::tr("Failed to write file %1!").arg(filePath));
        emit Finished();
        return;
    }

    state = UNPACKING;
    applicationsToRestart = GetApplicationsToRestart(params.branch, params.app);

    //remove application if was installed
    Application* localApp = configHolder->localConfig.GetApplication(params.branch, params.app);
    if (localApp != nullptr)
    {
        AppVersion* localVersion = localApp->GetVersion(0);
        if (localVersion != nullptr)
        {
            std::unique_ptr<BaseTask> task = appContext->CreateTask<RemoveApplicationTask>(configHolder, params.branch, params.app);
            appContext->taskManager.AddTask(std::move(task), notifier);
            return;
        }
    }
    //will not start remove task, can start install manually
    Install();
}

void InstallApplicationTask::Install()
{
    QString filePath = appContext->fileManager.GetTempDownloadFilePath(params.newVersion.url);

    QString appDirPath = LauncherUtils::GetApplicationDirectory(configHolder, appContext, params.branch, params.app, params.newVersion.isToolSet, false);
    state = POST_INSTALL;
    if (NeedUnpack())
    {
        std::unique_ptr<BaseTask> task = appContext->CreateTask<UnzipTask>(filePath, appDirPath);
        appContext->taskManager.AddTask(std::move(task), notifier);
        return;
    }
    else
    {
        QString newFilePath = appDirPath + appContext->fileManager.GetFileNameFromURL(params.newVersion.url);
        if (appContext->fileManager.MoveFileWithMakePath(filePath, newFilePath))
        {
            OnInstalled();
        }
        else
        {
            SetError(QObject::tr("Moving downloaded application failed"));
            emit Finished();
        }
    }
}

void InstallApplicationTask::OnInstalled()
{
    configHolder->localConfig.InsertApplication(params.branch, params.app, params.newVersion);
    configHolder->localConfig.UpdateApplicationsNames();
    configHolder->localConfig.SaveToFile(FileManager::GetLocalConfigFilePath());

    FileManager::DeleteDirectory(appContext->fileManager.GetTempDirectory());
    for (const QString& appToRestart : applicationsToRestart)
    {
        AppVersion* installedAppVersion = configHolder->localConfig.GetAppVersion(params.branch, appToRestart);
        appContext->taskManager.AddTask(appContext->CreateTask<RunApplicationTask>(configHolder, params.branch, appToRestart, installedAppVersion->id), notifier);
    }

    if (params.appToStart.isEmpty() == false)
    {
        Application* app = LauncherUtils::FindApplication(&configHolder->localConfig, params.branch, params.appToStart, true);
        if (app != nullptr)
        {
            AppVersion* version = LauncherUtils::FindVersion(app, QString(), true);
            if (version != nullptr)
            {
                appContext->taskManager.AddTask(appContext->CreateTask<RunApplicationTask>(configHolder, params.branch, app->id, version->id), notifier);
            }
        }
    }
    emit Finished();
}

QStringList InstallApplicationTask::GetApplicationsToRestart(const QString& branchID, const QString& appID)
{
    QStringList appList;
    const AppVersion* installedVersion = configHolder->localConfig.GetAppVersion(branchID, appID);

    if (installedVersion == nullptr)
    {
        return appList;
    }

    QStringList appNames;
    if (installedVersion->isToolSet)
    {
        QStringList toolsetApps = configHolder->localConfig.GetTranslatedToolsetApplications();
        for (auto iter = toolsetApps.begin(); iter != toolsetApps.end(); ++iter)
        {
            appNames << *iter;
        }
    }
    else
    {
        appNames << appID;
    }

    for (const QString& appName : appNames)
    {
        if (LauncherUtils::CanTryStopApplication(appName))
        {
            QString dirPath = LauncherUtils::GetApplicationDirectory(configHolder, appContext, branchID, appID, installedVersion->isToolSet, false);
            QString localPath = LauncherUtils::GetLocalAppPath(installedVersion, appID);
            if (appContext->appsCommandsSender.HostIsAvailable(dirPath + localPath))
            {
                appList << appName;
            }
        }
    }
    return appList;
}

bool InstallApplicationTask::NeedUnpack() const
{
    return params.newVersion.url.endsWith("zip");
}
