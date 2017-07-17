#include "Core/Tasks/ConsoleTasks/DownloadToDestinationTask.h"
#include "Core/Tasks/ConsoleTasks/ConsoleTasksCollection.h"

ConsoleTasksRegistrator registrator(DownloadToDestinationTask::staticMetaObject);

QCommandLineOption DownloadToDestinationTask::CreateOption() const
{
    return QCommandLineOption(QStringList() << "d"
                                            << "download",
                              tr("download file to <destination> from <application> <branch> <version>;\n"
                                 "if no <version> is set - will be used most recent version\n"
                                 "if no <branch> is set - will be used Stable"));
}
