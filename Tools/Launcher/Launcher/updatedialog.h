#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include "configparser.h"
#include "zipunpacker.h"
#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QQueue>
#include <QFile>
#include <QTreeWidgetItem>

namespace Ui {
class UpdateDialog;
}

class ApplicationManager;

struct UpdateTask
{
    UpdateTask(const QString & branch, const QString & app, const AppVersion & _version, bool _isSelfUpdate = false) :
        branchID(branch), appID(app), version(_version), isSelfUpdate(_isSelfUpdate) {}

    QString branchID;
    QString appID;
    AppVersion version;
    bool isSelfUpdate;
};

class UpdateDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit UpdateDialog(const QQueue<UpdateTask> & taskQueue, QWidget *parent = 0);
    ~UpdateDialog();

signals:
    void UpdateDownloadProgress(int value);
    void UpdateUnpackProgress(int value);
    void AppInstalled(const QString & branchID, const QString & appID, const AppVersion & version);

public slots:
    void OnCancelClicked();
    void UnpackProgress(int, int);
    void UnpackComplete();
    void UnpackError(int);

private slots:
    void NetworkError(QNetworkReply::NetworkError code);
    void DownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void DownloadFinished();
    void DownloadReadyRead();
    void StartNextTask();

private:
    void UpdateButton();

    void AddTopLogValue(const QString & log);
    void AddLogValue(const QString & log);
    void UpdateLastLogValue(const QString & log);
    void BreakLog();
    void CompleteLog();

    Ui::UpdateDialog *ui;

    QNetworkAccessManager * networkManager;
    QNetworkReply * currentDownload;

    int tasksCount;

    QFile outputFile;
    QQueue<UpdateTask> tasks;

    QTreeWidgetItem * currentLogItem;
    QTreeWidgetItem * currentTopLogItem;

    ZipUnpacker * unpacker;

    int lastErrorCode;
    QString lastErrorDesrc;
};

#endif // UPDATEDIALOG_H
