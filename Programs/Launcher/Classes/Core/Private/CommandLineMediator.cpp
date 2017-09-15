#include "Core/CommandLineMediator.h"
#include "Core/Tasks/ConsoleTasks/ConsoleTasksCollection.h"
#include "Core/Tasks/ConsoleTasks/ConsoleBaseTask.h"

#include <QMetaType>
#include <QMetaObject>

#include <QMap>
#include <QStringList>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>

void CommandLineMediator::Start(const QStringList& arguments)
{
    QCommandLineParser parser;

    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();

    ConsoleTasksCollection* collection = ConsoleTasksCollection::Instance();
    QMap<ConsoleBaseTask*, QCommandLineOption> tasks;
    for (const char* meta : collection->GetMetas())
    {
        int type = QMetaType::type(meta);
        void* task = QMetaType::create(type);
        ConsoleBaseTask* consoleTask = static_cast<ConsoleBaseTask*>(task);
        QCommandLineOption option = consoleTask->CreateOption();
        parser.addOption(option);
        tasks.insert(consoleTask, consoleTask->CreateOption());
    }

    if (!parser.parse(QCoreApplication::arguments()))
    {
        qInfo() << parser.errorText();
        return;
    }

    if (parser.isSet(versionOption))
    {
        parser.showVersion();
        return;
    }

    if (parser.isSet(helpOption))
    {
        parser.showHelp();
        return;
    }

    for (auto iter = tasks.begin(); iter != tasks.end(); ++iter)
    {
        if (parser.isSet(iter.value()))
        {
            iter.key()->Run(parser.positionalArguments());
        }
    }
}
