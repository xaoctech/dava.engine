#include "Core/Tasks/UpdateConfigTask.h"
#include "Core/Tasks/DownloadTask.h"

#include "Core/ApplicationManager.h"

#include "Gui/MainWindow.h"

#include <QBuffer>

UpdateConfigTask::UpdateConfigTask(ApplicationManager* appManager, const std::vector<QUrl>& urls)
    : AsyncChainTask(appManager)
{
    for (const QUrl& url : urls)
    {
        QBuffer* buffer = new QBuffer(this);
        buffer->open(QIODevice::ReadWrite);
        buffers[url] = buffer;
    }
}

QString UpdateConfigTask::GetDescription() const
{
    return QObject::tr("Updating configuration");
}

void UpdateConfigTask::Run()
{
    appManager->GetRemoteConfig()->Clear();
    QString description = QObject::tr("Downloading configuration");
    std::unique_ptr<BaseTask> task = appManager->CreateTask<DownloadTask>(description, buffers);
    Q_ASSERT(task != nullptr);
    appManager->AddTaskWithNotifier(std::move(task), notifier);
}

void UpdateConfigTask::OnFinished(const BaseTask* task)
{
    if (task->HasError())
    {
        appManager->GetMainWindow()->RefreshApps();
    }
    else
    {
        ConfigParser* remoteConfig = appManager->GetRemoteConfig();
        ConfigParser* localConfig = appManager->GetLocalConfig();
        Q_ASSERT(task->GetTaskType() == BaseTask::DOWNLOAD_TASK);

        const DownloadTask* downloadTask = static_cast<const DownloadTask*>(task);
        for (auto& bufferItem : buffers)
        {
            QBuffer* buffer = static_cast<QBuffer*>(bufferItem.second);
            Q_ASSERT(buffer->size() > 0);
            //old code, need to be refactored
            if (remoteConfig->ParseJSON(buffer->data(), this))
            {
                QString webPageUrl = remoteConfig->GetWebpageURL();
                if (webPageUrl.isEmpty() == false)
                {
                    localConfig->SetWebpageURL(webPageUrl);
                }
                localConfig->CopyStringsAndFavsFromConfig(remoteConfig);
                appManager->SaveLocalConfig();
            }
        }
        //old code, need to be refactored
        appManager->GetRemoteConfig()->UpdateApplicationsNames();
        appManager->GetLocalConfig()->UpdateApplicationsNames();
        appManager->SaveLocalConfig();
        appManager->GetMainWindow()->RefreshApps();
        appManager->CheckUpdates();
    }

    emit Finished();
}
