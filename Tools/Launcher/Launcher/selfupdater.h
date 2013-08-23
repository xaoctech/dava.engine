#ifndef SELFUPDATER_H
#define SELFUPDATER_H

#include "zipunpacker.h"
#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

namespace Ui {
class SelfUpdater;
}

class SelfUpdater : public QDialog
{
    Q_OBJECT
    
public:
    explicit SelfUpdater(const QString & arcUrl, QWidget *parent = 0);
    ~SelfUpdater();

signals:
    void StartUpdating();

private slots:
    void NetworkError(QNetworkReply::NetworkError code);
    void DownloadFinished();
    void OnStartUpdating();

private:
    Ui::SelfUpdater *ui;
    QString archiveUrl;

    QNetworkAccessManager * networkManager;
    QNetworkReply * currentDownload;

    ZipUnpacker * unpacker;

    int lastErrorCode;
    QString lastErrorDesrc;
};

#endif // SELFUPDATER_H
