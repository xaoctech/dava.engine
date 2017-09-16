#include "Core/Tasks/ConsoleTasks/SelfTestTask.h"
#include "Core/Tasks/ConsoleTasks/ConsoleTasksCollection.h"

#include <QDebug>

REGISTER_CLASS(SelfTestTask);

SelfTestTask::SelfTestTask()
{
}

SelfTestTask::~SelfTestTask()
{
}

QCommandLineOption SelfTestTask::CreateOption() const
{
    return QCommandLineOption(QStringList() << "s"
                                            << "selftest",
                              QObject::tr("starts the self-test process"));
}

void SelfTestTask::Run(const QStringList& arguments)
{
    qDebug() << arguments;
}
