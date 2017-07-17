#pragma once

#include "Core/Tasks/ConsoleTasks/ConsoleBaseTask.h"

class DownloadToDestinationTask : public ConsoleBaseTask
{
private:
    QCommandLineOption CreateOption() const override;
};
