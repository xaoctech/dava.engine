#pragma once

#include "Core/TaskManager.h"
#include "Core/UrlsHolder.h"
#include "Core/Tasks/BaseTask.h"

#include "Utils/FileManager.h"
#include "Utils/AppsCommandsSender.h"

struct ApplicationContext
{
    ApplicationContext();
    ApplicationContext(const ApplicationContext&) = delete;
    ApplicationContext(ApplicationContext&&) = delete;

    ApplicationContext& operator=(const ApplicationContext&) = delete;

    TaskManager taskManager;
    FileManager fileManager;
    AppsCommandsSender appsCommandsSender;
    UrlsHolder urlsHolder;

    template <typename T, typename... Arguments>
    std::unique_ptr<BaseTask> CreateTask(Arguments&&... args);
};

template <typename T, typename... Arguments>
std::unique_ptr<BaseTask> ApplicationContext::CreateTask(Arguments&&... args)
{
    return std::make_unique<T>(this, std::forward<Arguments>(args)...);
}
