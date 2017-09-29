#pragma once

#include "Core/GuiTasks/AsyncChainTask.h"

#include <QFile>

class ApplicationQuitController;

class SelfUpdateTask : public AsyncChainTask
{
public:
    SelfUpdateTask(ApplicationContext* appContext, ApplicationQuitController* quitController, const QString& url);

private:
    enum eState
    {
        LOADING,
        UNPACKING
    };

    QString GetDescription() const override;
    void Run() override;

    int GetSubtasksCount() const override;
    void OnFinished(const BaseTask* task) override;

    void OnLoaded(const BaseTask* task);
    void OnUnpacked();

    ApplicationQuitController* quitController = nullptr;
    QString url;
    QFile fileToWrite;
    eState state = LOADING;
};
