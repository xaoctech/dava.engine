#pragma once

#include "Core/Tasks/ConsoleTasks/ConsoleBaseTask.h"

class DownloadToDestinationTask : public ConsoleBaseTask
{
private:
    QCommandLineOption CreateOption() const override;
    void Run(const QStringList& arguments);
};

Q_DECLARE_METATYPE(DownloadToDestinationTask);
