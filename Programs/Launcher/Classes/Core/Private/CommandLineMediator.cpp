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
    for (const QMetaObject& meta : collection->GetMetas())
    {
        void* task = QMetaType::create(QMetaType::type(meta.className()));
        ConsoleBaseTask* consoleTask = static_cast<ConsoleBaseTask*>(task);
        tasks.insert(consoleTask, consoleTask->CreateOption());
    }

    if (!parser.parse(QCoreApplication::arguments()))
    {
        qInfo() << parser.errorText();
        return;
    }

    if (parser.isSet(versionOption))
    {
        qInfo() << parser.showVersion();
        return;
    }

    if (parser.isSet(helpOption))
    {
        qInfo() << parser.showHelp();
        return;
    }

    if (parser.isSet(versionOption))
        return CommandLineVersionRequested;

    if (parser.isSet(helpOption))
        return CommandLineHelpRequested;

    if (parser.isSet(nameServerOption))
    {
        const QString nameserver = parser.value(nameServerOption);
        query->nameServer = QHostAddress(nameserver);
        if (query->nameServer.isNull() || query->nameServer.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol)
        {
            *errorMessage = "Bad nameserver address: " + nameserver;
            return CommandLineError;
        }
    }

    if (parser.isSet(typeOption))
    {
        const QString typeParameter = parser.value(typeOption);
        const int type = typeFromParameter(typeParameter.toLower());
        if (type < 0)
        {
            *errorMessage = "Bad record type: " + typeParameter;
            return CommandLineError;
        }
        query->type = static_cast<QDnsLookup::Type>(type);
    }

    const QStringList positionalArguments = parser.positionalArguments();
    if (positionalArguments.isEmpty())
    {
        *errorMessage = "Argument 'name' missing.";
        return CommandLineError;
    }
    if (positionalArguments.size() > 1)
    {
        *errorMessage = "Several 'name' arguments specified.";
        return CommandLineError;
    }
    query->name = positionalArguments.first();

    return CommandLineOk;
}
