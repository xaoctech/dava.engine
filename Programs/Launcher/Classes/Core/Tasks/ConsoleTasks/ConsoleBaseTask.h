#pragma once

#include <QObject>
#include <QCommandLineOption>

//ConseoleTask can not be derived from BaseTask
//Because ConsoleTask can create ApplicationManager itself
class ConsoleBaseTask : public QObject
{
public:
    virtual QCommandLineOption CreateOption() const = 0;

protected:
};
