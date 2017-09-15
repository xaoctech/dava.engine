#pragma once

#include "Data/ConfigParser.h"
#include <QString>
#include <memory>

struct ApplicationContext;
struct ConfigHolder;

namespace LauncherUtils
{
QString GetAppName(const QString& appName, bool isToolSet);
QString GetLocalAppPath(const AppVersion* version, const QString& appID);
QString GetApplicationDirectory(const ConfigHolder* configHolder, const ApplicationContext* context, QString branchID, QString appID, bool isToolSet, bool mustExists);
bool CanTryStopApplication(const QString& applicationName);
}
