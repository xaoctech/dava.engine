#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

#include "configparser.h"
#include "defines.h"
#include "updatedialog.h"
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QQueue>

class ApplicationManager: public QObject
{
    Q_OBJECT
public:
    explicit ApplicationManager(QObject *parent = 0);
    ~ApplicationManager();

    void RefreshRemoteConfig();

    QString GetString(const QString & stringID);

    ConfigParser * GetLocalConfig();
    ConfigParser * GetRemoteConfig();

    void CheckUpdates(QQueue<UpdateTask> & tasks);

    void RunApplication(const QString & branchID, const QString & appID, const QString & versionID);
    bool RemoveApplication(const QString & branchID, const QString & appID, const QString & versionID);

    bool ShouldShowNews();
    void NewsShowed();

signals:
    void Refresh();

public slots:
    void OnAppInstalled(const QString & branchID, const QString & appID, const AppVersion & version);

private slots:
    void NetworkError(QNetworkReply::NetworkError code);
    void DownloadFinished();

private:
    void LoadLocalConfig(const QString & configPath);
    void DownloadRemoteConfig(const QUrl & url);

    QString localConfigFilePath;

    ConfigParser * localConfig;
    ConfigParser * remoteConfig;

    QNetworkAccessManager * networkManager;
    QNetworkReply * currentDownload;
};

#endif // APPLICATIONMANAGER_H
