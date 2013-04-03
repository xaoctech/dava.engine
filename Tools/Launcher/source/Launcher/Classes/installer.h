#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include "configDownload.h"
#include "selfupdater.h"
#include <QNetworkAccessManager>
#include "AppType.h"

//format sorfware list
class SoftWare
{
public:
    QString m_CurVersion;
    QString m_NewVersion;
    QString m_RunPath;
    bool m_bInstalling;
    QSet<QString> m_AvailableVersion;
    SoftWare() {
        m_bInstalling = false;
    }
};

class AvailableSoftWare
{
public:
    void Clear() {
        m_Stable.clear();
        m_Development.clear();
        m_Dependencies.clear();
    }

    typedef QMap<QString, SoftWare> SoftWareMap;
    SoftWareMap m_Stable;
    SoftWareMap m_Development;
    SoftWareMap m_Dependencies;
};

class Installer : public QObject
{
    Q_OBJECT
public:

    explicit Installer(QObject *parent = 0);
    void Init();
    
    void CheckForUpdate();
    bool Install(const QString& appName, const QString& appVersion, eAppType type);
    bool Delete(const QString& appName, eAppType type, bool force = false);
    QString GetRunPath(const QString& appName, eAppType type);
    bool Update(AvailableSoftWare::SoftWareMap softMap, eAppType type, bool force = false);
    bool AbortCurInstallation();

signals:
    void StartCheckForUpdate();
    void SoftWareAvailable(const AvailableSoftWare&);

    void StartDownload();
    void DownloadProgress(int);
    void DownloadFinished();
    
public slots:
    void UpdateConfigFinished(const AppsConfig&);
    void OnAppDownloaded();
    void OnDownloadProgress(qint64,qint64);

private:
    void FormatFromSetting(AvailableSoftWare::SoftWareMap& softMap, const AppsConfig::AppMap& setting);
    void FormatFromUpdate(AvailableSoftWare::SoftWareMap& softMap, const AppsConfig::AppMap& update);

#ifdef Q_OS_WIN
    int RunAndWaitForFinish(const QString& fileName, const QString& params = "");
#endif

    const AppsConfig::AppMap* GetAppMap(eAppType type) const;
    QString GetInstallPath(eAppType) const;

    void UpdateAvailableSoftware();
    
private:
    ConfigDownloader* m_pConfigDownloader;
    SelfUpdater* m_pSelfUpdater;
    AvailableSoftWare m_AvailableSoftWare;

    AppsConfig m_AppsConfig;
    QNetworkAccessManager* m_pNetworkManager;
    QNetworkReply* m_pReply;
};

#endif // UPDATER_H
