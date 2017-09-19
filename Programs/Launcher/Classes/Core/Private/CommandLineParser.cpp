#include "Core/CommandLineParser.h"
#include "Core/Tasks/ConsoleTasks/ConsoleTasksCollection.h"
#include "Core/Tasks/ConsoleTasks/ConsoleBaseTask.h"

#include <QMetaType>
#include <QMetaObject>

#include <QMap>
#include <QStringList>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>

void CommandLineParser::Parse(const QStringList& arguments)
{
    QCommandLineParser parser;

    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();

    ConsoleTasksCollection* collection = ConsoleTasksCollection::Instance();
    QMap<ConsoleBaseTask*, QCommandLineOption> tasks;
    for (const char* className : collection->GetClassNames())
    {
        int type = QMetaType::type(className);
        void* task = QMetaType::create(type);
        ConsoleBaseTask* consoleTask = static_cast<ConsoleBaseTask*>(task);
        QCommandLineOption option = consoleTask->CreateOption();
        parser.addOption(option);
        tasks.insert(consoleTask, consoleTask->CreateOption());
    }

    if (!parser.parse(arguments))
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
            QEventLoop loop;
            loop.exec();
        }
    }
}
