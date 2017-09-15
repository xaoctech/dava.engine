
#pragma once

#include <QList>
#include <QMetaObject>

class ConsoleTasksCollection
{
public:
    static ConsoleTasksCollection* Instance();

    void RegisterConsoleTask(const char* name);

    const QList<const char*>& GetMetas() const;

private:
    QList<const char*> tasks;
};

template <typename T>
struct ConsoleTasksRegistrator
{
    ConsoleTasksRegistrator(const char* name)
    {
        qRegisterMetaType<T>();
        ConsoleTasksCollection::Instance()->RegisterConsoleTask(name);
    }
};

#define REGISTER_CLASS(Type) ConsoleTasksRegistrator<Type> registrator(#Type);
