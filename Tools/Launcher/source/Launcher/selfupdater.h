#ifndef SELFUPDATER_H
#define SELFUPDATER_H

#include <QObject>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include "configDownload.h"

class SelfUpdater : public QObject
{
    Q_OBJECT
public:
    explicit SelfUpdater(QObject *parent = 0);
    virtual ~SelfUpdater();

signals:

private slots:
    void DownloadFinished();

public slots:
    void UpdatedConfigDownloaded(const AppsConfig& config);

private:
    QNetworkAccessManager* m_pNetworkManager;
    QNetworkReply* m_pReply;

    AppConfig m_UpdateLauncerConfig;
};

#endif // SELFUPDATER_H
