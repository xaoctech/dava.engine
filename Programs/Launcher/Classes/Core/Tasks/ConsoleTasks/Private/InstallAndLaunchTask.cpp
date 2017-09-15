#include "Core/Tasks/ConsoleTasks/InstallAndLaunchTask.h"
#include "Core/Tasks/ConsoleTasks/ConsoleTasksCollection.h"

#include "Core/Tasks/LoadLocalConfigTask.h"
#include "Core/Tasks/UpdateConfigTask.h"
#include "Core/Tasks/RunApplicationTask.h"
#include "Core/Tasks/InstallApplicationTask.h"
#include "Core/ApplicationContext.h"
#include "Core/ConfigHolder.h"
#include "Core/UrlsHolder.h"

#include "Gui/PreferencesDialog.h"

#include "Utils/Utils.h"

#include <QDebug>

#include <iostream>

REGISTER_CLASS(InstallAndLaunchTask);

InstallAndLaunchTask::InstallAndLaunchTask()
    : appContext(new ApplicationContext())
    , configHolder(new ConfigHolder())
{
}

InstallAndLaunchTask::~InstallAndLaunchTask()
{
    delete appContext;
    delete configHolder;
}

QCommandLineOption InstallAndLaunchTask::CreateOption() const
{
    return QCommandLineOption(QStringList() << "i"
                                            << "install",
                              QObject::tr("install <application> <branch> <version> and launch it;\n"
                                          "if no <version> is set - will be used most recent version\n"
                                          "if no <branch> is set - will be used Stable"
                                          "example -i QuickEd Stable 4389"));
}

void InstallAndLaunchTask::Run(const QStringList& arguments)
{
    ::LoadPreferences(appContext);

    if (arguments.isEmpty())
    {
        qDebug() << "error: pass application, please";
        exit(1);
    }

    appName = arguments.at(0);

    Receiver receiver;
    receiver.onFinished = [](const BaseTask* task) {
        if (task->HasError())
        {
            qDebug() << "error: " + task->GetError();
            exit(1);
        }
    };

    std::unique_ptr<BaseTask> loadConfigTask = appContext->CreateTask<LoadLocalConfigTask>(configHolder, FileManager::GetLocalConfigFilePath());
    appContext->taskManager.AddTask(std::move(loadConfigTask), receiver);

    UrlsHolder urlsHolder;

    std::unique_ptr<BaseTask> updateTask = appContext->CreateTask<UpdateConfigTask>(configHolder, urlsHolder.GetURLs());

    receiver.onStarted = [](const BaseTask* task) {
        qDebug() << task->GetDescription();
    };
    receiver.onProgress = [](const BaseTask* task, quint32 progress) {
        std::cout << "progress: " << progress << "\r";
    };
    receiver.onFinished = [arguments, this](const BaseTask* task) {
        if (dynamic_cast<const UpdateConfigTask*>(task) != nullptr)
        {
            if (task->HasError())
            {
                qDebug() << "error: " + task->GetError();
                exit(1);
            }
            else
            {
                OnUpdateConfigFinished(arguments);
            }
        }
    };
    appContext->taskManager.AddTask(std::move(updateTask), receiver);
}

void InstallAndLaunchTask::OnUpdateConfigFinished(const QStringList& arguments)
{
    QString branchName = "Stable";
    if (arguments.size() >= 2)
    {
        branchName = arguments.at(1);
    }

    QString versionName = "recent";
    if (arguments.size() >= 3)
    {
        versionName = arguments.at(2);
    }

    AppVersion* localVersion = LauncherUtils::FindVersion(&configHolder->localConfig, branchName, appName, versionName, true);
    AppVersion* remoteVersion = LauncherUtils::FindVersion(&configHolder->remoteConfig, branchName, appName, versionName, true);

    Receiver runReceiver;
    runReceiver.onFinished = [this](const BaseTask* task) {
        if (task->HasError())
        {
            qDebug() << "error: " + task->GetError();
            exit(1);
        }
        else
        {
            qDebug() << "success";
            exit(0);
        }
    };

    if (remoteVersion == nullptr)
    {
        if (localVersion == nullptr)
        {
            qDebug() << "error: application" << appName << "in branch" << branchName << "with version" << versionName << "not found on local and on remote";
            exit(1);
        }
        else
        {
            Branch* branch = LauncherUtils::FindBranch(&configHolder->localConfig, branchName, false);
            Application* app = LauncherUtils::FindApplication(branch, appName, false);
            std::unique_ptr<BaseTask> runTask = appContext->CreateTask<RunApplicationTask>(configHolder, branch->id, app->id, localVersion->id);
            appContext->taskManager.AddTask(std::move(runTask), runReceiver);
        }
    }
    else
    {
        if (localVersion != nullptr)
        {
            if (localVersion->id == remoteVersion->id)
            {
                Branch* branch = LauncherUtils::FindBranch(&configHolder->localConfig, branchName, false);
                Application* app = LauncherUtils::FindApplication(branch, appName, false);
                std::unique_ptr<BaseTask> runTask = appContext->CreateTask<RunApplicationTask>(configHolder, branch->id, app->id, localVersion->id);
                appContext->taskManager.AddTask(std::move(runTask), runReceiver);
                //this return must be not called
                return;
            }
        }

        Branch* branch = LauncherUtils::FindBranch(&configHolder->remoteConfig, branchName, false);
        Application* app = LauncherUtils::FindApplication(branch, appName, false);

        InstallApplicationParams params;
        params.branch = branch->id;
        params.app = app->id;
        params.newVersion = *remoteVersion;
        params.appToStart = appName;

        Receiver installReceiver;
        installReceiver.onStarted = [](const BaseTask* task) {
            qDebug() << task->GetDescription();
        };
        installReceiver.onProgress = [](const BaseTask* task, quint32 progress) {
            std::cout << "progress: " << progress << "\r";
        };
        installReceiver.onFinished = [this](const BaseTask* task) {
            if (dynamic_cast<const InstallApplicationTask*>(task) != nullptr)
            {
                if (task->HasError())
                {
                    qDebug() << "error: " + task->GetError();
                    exit(1);
                }
                else
                {
                    qDebug() << "success";
                    exit(0);
                }
            }
        };

        std::unique_ptr<BaseTask> installTask = appContext->CreateTask<InstallApplicationTask>(configHolder, params);
        appContext->taskManager.AddTask(std::move(installTask), installReceiver);
    }
}
