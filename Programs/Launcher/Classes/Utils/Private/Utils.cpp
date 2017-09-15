#include "Utils/Utils.h"
#include "Utils/FileManager.h"

#include "Core/ApplicationContext.h"
#include "Core/ConfigHolder.h"

#include <QRegularExpression>
#include <QFile>

namespace LauncherUtils
{
QString RemoveWhitespace(const QString& str)
{
    QString replacedStr = str;
    QRegularExpression spaceRegex("\\s+");
    replacedStr.replace(spaceRegex, "");
    return replacedStr;
}

QString GetAppName(const QString& appName, bool isToolSet)
{
    return isToolSet ? "Toolset" : appName;
}

QString GetLocalAppPath(const AppVersion* version, const QString& appID)
{
    Q_ASSERT(!appID.isEmpty());
    if (version == nullptr)
    {
        return QString();
    }
    QString runPath = version->runPath;
    if (runPath.isEmpty())
    {
        QString correctID = RemoveWhitespace(appID);
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

QString GetApplicationDirectory_kostil(const ApplicationContext* context, const QString& branchID, const QString& appID)
{
    QString path = context->fileManager.GetBaseAppsDirectory() + branchID + "/" + appID + "/";
    return path;
}

QString GetApplicationDirectory(const ConfigHolder* configHolder, const ApplicationContext* context, QString branchID, QString appID, bool isToolSet, bool mustExists)
{
    appID = LauncherUtils::GetAppName(appID, isToolSet);

    branchID = RemoveWhitespace(branchID);
    appID = RemoveWhitespace(appID);

    //try to get right path
    QString runPath = context->fileManager.GetApplicationDirectory(branchID, appID);
    if (QFile::exists(runPath))
    {
        return runPath;
    }

    //try to get old ugly path with a bug on "/" symbol
    QString tmpRunPath = GetApplicationDirectory_kostil(context, branchID, appID);
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
            newRunPath = GetApplicationDirectory_kostil(context, branchKey, appKey);
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

bool CanTryStopApplication(const QString& applicationName)
{
    return applicationName.contains("assetcacheserver", Qt::CaseInsensitive);
}
}
