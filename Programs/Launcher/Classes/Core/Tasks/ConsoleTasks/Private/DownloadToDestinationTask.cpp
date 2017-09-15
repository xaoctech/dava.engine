#include "Core/Tasks/ConsoleTasks/DownloadToDestinationTask.h"
#include "Core/Tasks/ConsoleTasks/ConsoleTasksCollection.h"

#include <QDebug>

REGISTER_CLASS(DownloadToDestinationTask);

QCommandLineOption DownloadToDestinationTask::CreateOption() const
{
    return QCommandLineOption(QStringList() << "d"
                                            << "download",
                              QObject::tr("download file to <destination> from <application> <branch> <version>;\n"
                                          "if no <version> is set - will be used most recent version\n"
                                          "if no <branch> is set - will be used Stable"));
}

void DownloadToDestinationTask::Run(const QStringList& arguments)
{
    if (arguments.size() < 2)
    {
        qCritical() << QObject::tr("Pass destination and application please");
    }

    qDebug() << arguments;
}
