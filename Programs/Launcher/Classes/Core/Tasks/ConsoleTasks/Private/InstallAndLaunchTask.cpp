#include "Core/Tasks/ConsoleTasks/InstallAndLaunchTask.h"
#include "Core/Tasks/ConsoleTasks/ConsoleTasksCollection.h"

#include <QDebug>

REGISTER_CLASS(InstallAndLaunchTask);

QCommandLineOption InstallAndLaunchTask::CreateOption() const
{
    return QCommandLineOption(QStringList() << "i"
                                            << "install",
                              QObject::tr("install <application> <branch> <version> and launch it;\n"
                                          "if no <version> is set - will be used most recent version\n"
                                          "if no <branch> is set - will be used Stable"));
}

void InstallAndLaunchTask::Run(const QStringList& arguments)
{
    if (arguments.size() < 1)
    {
        qCritical() << QObject::tr("Pass an application name please");
    }

    qDebug() << arguments;
}
