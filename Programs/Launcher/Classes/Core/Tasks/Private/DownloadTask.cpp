#include "Core/Tasks/DownloadTask.h"

#include <QIODevice>

DownloadTask::DownloadTask(ApplicationManager* appManager, const QString& description_, const std::map<QUrl, QIODevice*>& buffers_)
    : BaseTask(appManager)
    , description(description_)
    , buffers(buffers_)
{
}

DownloadTask::DownloadTask(ApplicationManager* appManager, const QString& description_, const QUrl& url, QIODevice* writeBuffer)
    : BaseTask(appManager)
    , description(description_)
{
    buffers[url] = writeBuffer;
}

BaseTask::eTaskType DownloadTask::GetTaskType() const
{
    return DOWNLOAD_TASK;
}

QString DownloadTask::GetDescription() const
{
    return description;
}

void DownloadTask::AddLoadedData(const QUrl& url, QByteArray& data)
{
    Q_ASSERT(buffers.find(url) != buffers.end());
    buffers[url]->write(data);
}

std::vector<QUrl> DownloadTask::GetUrls() const
{
    std::vector<QUrl> urls;
    urls.reserve(buffers.size());
    for (const auto& item : buffers)
    {
        urls.push_back(item.first);
    }
    return urls;
}

bool DownloadTask::IsCancelled() const
{
    return isCancelled;
}

void DownloadTask::SetCancelled(bool cancelled)
{
    isCancelled = cancelled;
}
