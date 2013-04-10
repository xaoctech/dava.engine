#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QMap>
#include "configDownload.h"
#include "AppType.h"

class Settings : public QObject
{
    Q_OBJECT

public:
    explicit Settings(QObject *parent = 0);
    
    static Settings* GetInstance();
    static QString GetVersion(const QString& strVersion);

    void Init();

    QString GetInstalledAppVersion(const QString& appName, eAppType type);

    int GetUpdateTimerInterval() const {return m_nUpdateTimer;}
    void SetLauncherVersion(const QString& version);
    QString GetLauncherVersion() const;
    void SetLauncherUrl(const QString& url);
    QString GetLauncherUrl() const;

    const AppsConfig& GetCurrentConfig() const;
    void UpdateConfig(const AppsConfig&);
    void SaveConfig() {UpdateInitConfig();};

signals:

public slots:

private:
    void ParseInitConfig();
    void UpdateInitConfig();

    void ParseAppConfig(const YAML::Node* pNode, const char* appType, AppsConfig::AppMap& appMap);
    void EmitAppConfig(YAML::Emitter& emitter, const char* appType, const AppsConfig::AppMap& appMap);
    
private:
    static Settings* m_spInstance;

    int m_nUpdateTimer;
    AppsConfig m_Config;
};

#endif // SETTINGS_H
