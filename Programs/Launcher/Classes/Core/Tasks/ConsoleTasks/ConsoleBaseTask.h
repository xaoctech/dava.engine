#pragma once

#include <QObject>
#include <QCommandLineOption>

//ConseoleTask can not be derived from BaseTask
//Because ConsoleTask can create ApplicationManager itself
class ConsoleBaseTask
{
public:
    virtual ~ConsoleBaseTask();
    virtual QCommandLineOption CreateOption() const = 0;
    virtual void Run(const QStringList& arguments) = 0;
};
