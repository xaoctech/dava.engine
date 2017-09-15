#pragma once

#include "Core/TaskManager.h"
#include "Core/Tasks/BaseTask.h"

#include "Utils/FileManager.h"
#include "Utils/AppsCommandsSender.h"

struct ApplicationContext
{
    TaskManager taskManager;
    FileManager fileManager;
    AppsCommandsSender appsCommandsSender;
    template <typename T, typename... Arguments>
    std::unique_ptr<BaseTask> CreateTask(Arguments&&... args);
};

template <typename T, typename... Arguments>
std::unique_ptr<BaseTask> ApplicationContext::CreateTask(Arguments&&... args)
{
    return std::make_unique<T>(this, std::forward<Arguments>(args)...);
}
