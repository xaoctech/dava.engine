#include "Utils/Utils.h"
#include "Utils/FileManager.h"

#include "Data/ConfigParser.h"

#include "Core/ApplicationContext.h"
#include "Core/ConfigHolder.h"

#include <QDebug>
#include <QRegularExpression>
#include <QFile>

#include <assert.h>

namespace LauncherUtilsDetails
{
template <typename T>
T* FindItemWithName(QList<T>& cont, const QString& name, bool silent)
{
    QList<T*> items;
    for (T& item : cont)
    {
        if (item.id.contains(name, Qt::CaseInsensitive))
        {
            items.push_back(&item);
        }
    }
    if (items.size() == 1)
    {
        return items.front();
    }
    else if (items.isEmpty())
    {
        if (silent)
        {
            return nullptr;
        }
        else
        {
            qDebug() << "error: no items found for branch " << name;
            exit(1);
        }
    }
    else
    {
        QList<T*> foundItems;
        for (T* item : items)
        {
            if (item->id.compare(name, Qt::CaseInsensitive) == 0)
            {
                foundItems.push_back(item);
            }
        }
        if (foundItems.isEmpty())
        {
            if (silent)
            {
                return nullptr;
            }
            else
            {
                qDebug() << "error: found more than one item with name" << name << ", but no one matches:b";
                for (T* item : foundItems)
                {
                    qDebug() << item->id;
                }
                exit(1);
            }
        }
        else if (foundItems.size() == 1)
        {
            return foundItems.front();
        }
        else
        {
            foundItems.clear();
            for (T* item : items)
            {
                if (item->id.compare(name, Qt::CaseSensitive) == 0)
                {
                    foundItems.push_back(item);
                }
            }
            if (foundItems.size() == 1)
            {
                return foundItems.front();
            }
            else
            {
                if (silent)
                {
                    return nullptr;
                }
                else
                {
                    qDebug() << "error: internal error! More than one item with name" << name;
                    exit(1);
                }
            }
        }
    }
}

QString RemoveWhitespace(const QString& str)
{
    QString replacedStr = str;
    QRegularExpression spaceRegex("\\s+");
    replacedStr.replace(spaceRegex, "");
    return replacedStr;
}

QString GetApplicationDirectory_kostil(const ApplicationContext* context, const QString& branchID, const QString& appID)
{
    QString path = context->fileManager.GetBaseAppsDirectory() + branchID + "/" + appID + "/";
    return path;
}
}

QString LauncherUtils::GetAppName(const QString& appName, bool isToolSet)
{
    return isToolSet ? "Toolset" : appName;
}

QString LauncherUtils::GetLocalAppPath(const AppVersion* version, const QString& appID)
{
    Q_ASSERT(!appID.isEmpty());
    if (version == nullptr)
    {
        return QString();
    }
    QString runPath = version->runPath;
    if (runPath.isEmpty())
    {
        QString correctID = LauncherUtilsDetails::RemoveWhitespace(appID);
#ifdef Q_OS_WIN
        runPath = correctID + ".exe";
#elif defined(Q_OS_MAC)
        runPath = correctID + ".app";
#else
#error "unsupported platform"
#endif //platform
    }
    return runPath;
}

QString LauncherUtils::GetApplicationDirectory(const ConfigHolder* configHolder, const ApplicationContext* context, QString branchID, QString appID, bool isToolSet, bool mustExists)
{
    appID = LauncherUtils::GetAppName(appID, isToolSet);

    branchID = LauncherUtilsDetails::RemoveWhitespace(branchID);
    appID = LauncherUtilsDetails::RemoveWhitespace(appID);

    //try to get right path
    QString runPath = context->fileManager.GetApplicationDirectory(branchID, appID);
    if (QFile::exists(runPath))
    {
        return runPath;
    }

    //try to get old ugly path with a bug on "/" symbol
    QString tmpRunPath = LauncherUtilsDetails::GetApplicationDirectory_kostil(context, branchID, appID);
    if (QFile::exists(tmpRunPath))
    {
        return tmpRunPath;
    }
    //we can have old branch name or old app name
    QList<QString> branchKeys = configHolder->localConfig.GetStrings().keys(branchID);
    //it can be combination of old and new names
    branchKeys.append(branchID);
    for (const QString& branchKey : branchKeys)
    {
        QList<QString> appKeys = configHolder->localConfig.GetStrings().keys(appID);
        appKeys.append(appID);
        for (const QString& appKey : appKeys)
        {
            QString newRunPath = context->fileManager.GetApplicationDirectory(branchKey, appKey);
            if (QFile::exists(newRunPath))
            {
                return newRunPath;
            }
            newRunPath = LauncherUtilsDetails::GetApplicationDirectory_kostil(context, branchKey, appKey);
            if (QFile::exists(newRunPath))
            {
                return newRunPath;
            }
        }
    }
    //we expect that folder exists
    //or we just downloaded it and did not find original folder? make new folder with a correct name
    if (mustExists)
    {
        return "";
    }
    else
    {
        FileManager::MakeDirectory(runPath);
        return runPath;
    }
}

bool LauncherUtils::CanTryStopApplication(const QString& applicationName)
{
    return applicationName.contains("assetcacheserver", Qt::CaseInsensitive);
}

Branch* LauncherUtils::FindBranch(ConfigParser* config, const QString& branchName, bool silent)
{
    assert(config != nullptr);
    return LauncherUtilsDetails::FindItemWithName(config->GetBranches(), branchName, silent);
}

Application* LauncherUtils::FindApplication(Branch* branch, QString appName, bool silent)
{
    assert(branch != nullptr);
    if (appName.compare("QE", Qt::CaseInsensitive) == 0)
    {
        appName = "QuickEd";
    }

    return LauncherUtilsDetails::FindItemWithName(branch->applications, appName, silent);
}

AppVersion* LauncherUtils::FindVersion(Application* app, QString versionName, bool silent)
{
    assert(app != nullptr);
    if (versionName.isEmpty() || versionName.compare("recent", Qt::CaseInsensitive) == 0)
    {
        if (app->versions.isEmpty())
        {
            if (silent)
            {
                return nullptr;
            }
            else
            {
                qDebug() << "Error: no versions in application" << app->id;
                exit(1);
            }
        }
        else
        {
            return &app->versions.front();
        }
    }

    return LauncherUtilsDetails::FindItemWithName(app->versions, versionName, silent);
}

Application* LauncherUtils::FindApplication(ConfigParser* config, const QString& branchName, const QString& appName, bool silent)
{
    Branch* branch = FindBranch(config, branchName, silent);
    if (branch != nullptr)
    {
        return FindApplication(branch, appName, silent);
    }
    return nullptr;
}

AppVersion* LauncherUtils::FindVersion(Branch* branch, const QString& appName, const QString& versionName, bool silent)
{
    Application* app = FindApplication(branch, appName, silent);
    if (app != nullptr)
    {
        return FindVersion(app, versionName, silent);
    }

    return nullptr;
}

AppVersion* LauncherUtils::FindVersion(ConfigParser* config, const QString& branchName, const QString& appName, const QString& versionName, bool silent)
{
    Branch* branch = FindBranch(config, branchName, silent);
    if (branch != nullptr)
    {
        return FindVersion(branch, appName, versionName, silent);
    }
    return nullptr;
}
