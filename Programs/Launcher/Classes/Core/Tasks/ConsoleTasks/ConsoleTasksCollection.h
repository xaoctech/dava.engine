#pragma once

#include <QList>
#include <QMetaObject>

class ConsoleTasksCollection
{
public:
    static ConsoleTasksCollection* Instance();

    void RegisterConsoleTask(const QMetaObject& meta);

    const QList<QMetaObject>& GetMetas() const;

private:
    QList<QMetaObject> tasks;

    static ConsoleTasksCollection* self;
};

struct ConsoleTasksRegistrator
{
    ConsoleTasksRegistrator(const QMetaObject& metaObject);
};
