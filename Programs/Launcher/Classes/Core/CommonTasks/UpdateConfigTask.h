#pragma once

#include "Core/GuiTasks/AsyncChainTask.h"

#include <QUrl>
#include <QString>

class QIODevice;
struct ConfigHolder;

class UpdateConfigTask : public AsyncChainTask
{
public:
    UpdateConfigTask(ApplicationContext* appContext, ConfigHolder* configHolder, const std::vector<QUrl>& urls);

private:
    QString GetDescription() const override;
    void Run() override;

    void OnFinished(const BaseTask* task) override;

    std::map<QUrl, QIODevice*> buffers;
    ConfigHolder* configHolder = nullptr;
};
