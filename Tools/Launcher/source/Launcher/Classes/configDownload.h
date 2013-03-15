#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QString>
#include <QUrl>
#include <QMap>
#include <QStringList>
#include "AppType.h"
#include "yaml-cpp/yaml.h"

class AppConfig {
public:
    QString m_Name;
    QString m_Version;
    QUrl m_Url;
    QString m_RunPath;
    QStringList m_InstalledFiles;
    QString m_InstallCmd;
    QString m_UninstallCmd;
    QString m_InstallParams;
    QString m_UninstallParams;
    int m_nSuccessInstallCode;

    AppConfig() {
        m_nSuccessInstallCode = 0;
    }

    void Clear() {
        m_Name.clear();
        m_Version.clear();
        m_Url.clear();
        m_RunPath.clear();
        m_InstalledFiles.clear();
        m_InstallCmd.clear();
        m_UninstallCmd.clear();
        m_InstallParams.clear();
        m_UninstallParams.clear();
    }
};

class AppsConfig {
public:
    AppConfig m_Launcher;

    typedef QMap<QString, AppConfig> AppVersion;    //version, app config
    typedef QMap<QString, AppVersion> AppMap;
    AppMap m_Stable;
    AppMap m_Development;
    AppMap m_Dependencies;

    void Clear();

    AppMap* GetAppMap(eAppType type) {
        switch (type) {
        case eAppTypeStable: {
            return &m_Stable;
        }break;
        case eAppTypeDevelopment: {
            return &m_Development;
        }break;
        case eAppTypeDependencies: {
            return &m_Dependencies;
        }break;
        }
        return NULL;
    }
};

class ConfigDownloader : public QObject
{
    Q_OBJECT
public:
    explicit ConfigDownloader(QObject *parent = 0);
    
    void Init();

signals:
    void UpdateConfigFinished(const AppsConfig&);
    
public slots:
    void UpdateConfig();
    void ConfigDownloaded();

private:
    enum eDownloadState {
        eDownloadStateInactive,
        eDownloadStateLauncher,
        eDownloadStateStable,
        eDownloadStateDevelopment,
        eDownloadStateDependencies
    };

    void SetState(eDownloadState newState);
    void ParseAppData(AppsConfig::AppMap& appMap, const YAML::Node* pNode);
    
private:
    eDownloadState m_State;

    QNetworkAccessManager* m_pManager;
    QNetworkReply* m_pReply;

    AppsConfig m_UpdatedConfig;
    QString m_StableUrl;
    QString m_DevelopmentUrl;
    QString m_DependenciesUrl;
};

#endif // CONFIGURATION_H
