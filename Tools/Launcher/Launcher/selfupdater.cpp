#include "selfupdater.h"
#include "ui_selfupdater.h"
#include "filemanager.h"
#include "processhelper.h"
#include "errormessanger.h"
#include <QProcess>

SelfUpdater::SelfUpdater(const QString & arcUrl, QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::CustomizeWindowHint),
    ui(new Ui::SelfUpdater),
    archiveUrl(arcUrl),
    currentDownload(0),
    unpacker(0),
    lastErrorCode(0)
{
    ui->setupUi(this);

    networkManager = new QNetworkAccessManager();
    unpacker = new ZipUnpacker();

    connect(this, SIGNAL(StartUpdating()), this, SLOT(OnStartUpdating()));

    emit StartUpdating();
}

SelfUpdater::~SelfUpdater()
{
    SafeDelete(ui);

    SafeDelete(networkManager);
    SafeDelete(currentDownload);
    SafeDelete(unpacker);
}

void SelfUpdater::OnStartUpdating()
{
    currentDownload = networkManager->get(QNetworkRequest(QUrl(archiveUrl)));

    connect(currentDownload, SIGNAL(finished()), this, SLOT(DownloadFinished()));
    connect(currentDownload, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(NetworkError(QNetworkReply::NetworkError)));
}

void SelfUpdater::NetworkError(QNetworkReply::NetworkError code)
{
    lastErrorDesrc = currentDownload->errorString();
    lastErrorCode = code;

    currentDownload->deleteLater();
    currentDownload = 0;
}

void SelfUpdater::DownloadFinished()
{
    if(currentDownload)
    {
        FileManager::Instance()->ClearTempDirectory();

        const QString & archiveFilePath = FileManager::Instance()->GetTempDownloadFilepath();
        const QString & tempDir = FileManager::Instance()->GetTempDirectory();
        const QString & appDir = FileManager::Instance()->GetLauncherDirectory();
        const QString & selfUpdateDir = FileManager::Instance()->GetSelfUpdateTempDirectory();

        QFile archiveFile(archiveFilePath);
        archiveFile.open(QFile::WriteOnly);
        archiveFile.write(currentDownload->readAll());
        archiveFile.close();

        currentDownload->deleteLater();
        currentDownload = 0;

        unpacker->UnZipFile(archiveFilePath, selfUpdateDir);

        FileManager::Instance()->MoveFilesOnlyToDirectory(appDir, tempDir);
        FileManager::Instance()->MoveFilesOnlyToDirectory(selfUpdateDir, appDir);
        FileManager::Instance()->DeleteDirectory(selfUpdateDir);

        ErrorMessanger::Instance()->ShowNotificationDlg("Launcher was updated. Please, relaunch application.");

        qApp->exit();
    }
    else if(lastErrorCode != QNetworkReply::OperationCanceledError)
    {
        setResult(QDialog::Rejected);
        ErrorMessanger::Instance()->ShowErrorMessage(ErrorMessanger::ERROR_NETWORK, lastErrorCode, lastErrorDesrc);
        close();
    }
}
