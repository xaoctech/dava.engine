#pragma once

#include "Core/Tasks/ConsoleTasks/ConsoleBaseTask.h"

struct ApplicationContext;
struct ConfigHolder;

class InstallAndLaunchTask : public ConsoleBaseTask
{
public:
    InstallAndLaunchTask();
    ~InstallAndLaunchTask() override;

private:
    QCommandLineOption CreateOption() const override;
    void Run(const QStringList& arguments) override;
    void OnUpdateConfigFinished(const QStringList& arguments);

    ApplicationContext* appContext;
    ConfigHolder* configHolder;

    QString appName;
};

Q_DECLARE_METATYPE(InstallAndLaunchTask);
