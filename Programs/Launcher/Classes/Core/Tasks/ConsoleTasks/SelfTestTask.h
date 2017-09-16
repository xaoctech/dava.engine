#pragma once

#include "Core/Tasks/ConsoleTasks/ConsoleBaseTask.h"

class SelfTestTask : public ConsoleBaseTask
{
public:
    SelfTestTask();
    ~SelfTestTask() override;

private:
    QCommandLineOption CreateOption() const override;
    void Run(const QStringList& arguments) override;
};

Q_DECLARE_METATYPE(SelfTestTask);
