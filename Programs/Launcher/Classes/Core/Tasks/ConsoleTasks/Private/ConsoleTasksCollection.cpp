#include "Core/Tasks/ConsoleTasks/ConsoleTasksCollection.h"

ConsoleTasksCollection* ConsoleTasksCollection::self;

ConsoleTasksCollection* ConsoleTasksCollection::Instance()
{
    return self;
}

void ConsoleTasksCollection::RegisterConsoleTask(const QMetaObject& meta)
{
    tasks.append(meta);
}

const QList<QMetaObject>& ConsoleTasksCollection::GetMetas() const
{
    return tasks;
}

ConsoleTasksRegistrator::ConsoleTasksRegistrator(const QMetaObject& metaObject)
{
    ConsoleTasksCollection::Instance()->RegisterConsoleTask(metaObject);
}
