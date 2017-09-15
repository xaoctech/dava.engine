#pragma once

#include "Data/ConfigParser.h"
#include <QString>
#include <memory>

struct ApplicationContext;
struct ConfigHolder;

class ConfigParser;
struct Branch;
struct Application;
struct AppVersion;

namespace LauncherUtils
{
QString GetAppName(const QString& appName, bool isToolSet);
QString GetLocalAppPath(const AppVersion* version, const QString& appID);
QString GetApplicationDirectory(const ConfigHolder* configHolder, const ApplicationContext* context, QString branchID, QString appID, bool isToolSet, bool mustExists);
bool CanTryStopApplication(const QString& applicationName);

Branch* FindBranch(ConfigParser* config, const QString& branchName, bool silent);
Application* FindApplication(Branch* branch, QString appName, bool silent);
Application* FindApplication(ConfigParser* config, const QString& branchName, const QString& appName, bool silent);
AppVersion* FindVersion(Application* app, QString versionName, bool silent);
AppVersion* FindVersion(Branch* branch, const QString& appName, const QString& versionName, bool silent);
AppVersion* FindVersion(ConfigParser* config, const QString& branchName, const QString& appName, const QString& versionName, bool silent);
}
